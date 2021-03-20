#include "libserialport.h"
#include "lwrb/lwrb.h"
#include "ros/ros.h"
#include "slff/buzzer.h"
#include "slff/define.h"
#include "slff/misc.h"
#include "slff/rs232_baudrate.h"
#include "slff/rs232_data.h"
#include "std_msgs/UInt16.h"
#include "std_msgs/UInt32.h"
#include "std_msgs/UInt8.h"

#define BUFFLEN 512

//=====Prototype
void cllbck_tim_100hz(const ros::TimerEvent &event);
void cllbck_tim_101hz(const ros::TimerEvent &event);

void cllbck_sub_opto_out(const std_msgs::UInt8ConstPtr &msg);
void cllbck_sub_led_out(const std_msgs::UInt16ConstPtr &msg);
void cllbck_sub_buzzer(const slff::buzzerConstPtr &msg);
void cllbck_sub_rs232_baudrate(const slff::rs232_baudrateConstPtr &msg);
void cllbck_sub_rs232_tx(const slff::rs232_dataConstPtr &msg);

int expansion_init();
int expansion_routine();
void expansion_parser(uint8_t data);

uint32_t get_epoch();

//=====Parameter
bool expansion_active;
std::string expansion_port;
int expansion_baud;
//=====Timer
ros::Timer tim_100hz;
ros::Timer tim_101hz;
//=====Subscriber
ros::Subscriber sub_opto_out;
ros::Subscriber sub_led_out;
ros::Subscriber sub_buzzer;
ros::Subscriber sub_rs232_baudrate;
ros::Subscriber sub_rs232_tx;
//=====Publisher
ros::Publisher pub_opto_in;
ros::Publisher pub_rs232_rx;
ros::Publisher pub_log;
//=====Help
misc help;

//=====Serial Connection
struct sp_port *serial_port;
double serial_timer;
uint8_t serial_data;
uint8_t tx_buffer[BUFFLEN * 6];
uint8_t rx_buffer[BUFFLEN * 6];
uint16_t tx_len;
uint16_t rx_len;

//=====Frame
const uint8_t frame_header[3] = {'$', '$', '$'};

//=====Data
uint32_t epoch_in = 0;
uint32_t epoch_out = 0;

uint8_t opto_out = 0x00;
uint8_t opto_in = 0x3f;
uint16_t led_out = 0x0000;

uint16_t buzzer_waktu;
uint16_t buzzer_jumlah;

struct RS232
{
    uint32_t baudrate;

    lwrb_t tx_ringbuffer;
    uint8_t tx_buffer[BUFFLEN];
    uint16_t tx_len;

    lwrb_t rx_ringbuffer;
    uint8_t rx_buffer[BUFFLEN];
    uint16_t rx_len;
} rs232[4];

uint16_t rs232_tx_len;
uint16_t rs232_rx_len;

int main(int argc, char **argv)
{
    ros::init(argc, argv, "expansion");

    ros::NodeHandle NH;
    ros::MultiThreadedSpinner MTS;

    //=====Parameter
    NH.getParam("expansion/active", expansion_active);
    NH.getParam("expansion/port", expansion_port);
    NH.getParam("expansion/baud", expansion_baud);
    //=====Timer
    tim_100hz = NH.createTimer(ros::Duration(0.01), cllbck_tim_100hz);
    tim_101hz = NH.createTimer(ros::Duration(0.01), cllbck_tim_101hz);
    //=====Subscriber
    sub_opto_out = NH.subscribe("expansion/opto_out", 0, cllbck_sub_opto_out);
    sub_led_out = NH.subscribe("expansion/led_out", 0, cllbck_sub_led_out);
    sub_buzzer = NH.subscribe("expansion/buzzer", 0, cllbck_sub_buzzer);
    sub_rs232_baudrate = NH.subscribe("expansion/rs232_baudrate", 0, cllbck_sub_rs232_baudrate);
    sub_rs232_tx = NH.subscribe("expansion/rs232_tx", 0, cllbck_sub_rs232_tx);
    //=====Publisher
    pub_opto_in = NH.advertise<std_msgs::UInt8>("expansion/opto_in", 0);
    pub_rs232_rx = NH.advertise<slff::rs232_data>("expansion/rs232_rx", 0);
    //=====Help
    help.init(&NH);

    if (expansion_init() == -1)
        ros::shutdown();

    MTS.spin();
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void cllbck_tim_100hz(const ros::TimerEvent &event)
{
    if (expansion_routine() == -1)
        ros::shutdown();
}

void cllbck_tim_101hz(const ros::TimerEvent &event)
{
    if (expansion_active)
    {
        tx_len = 0;

        //---------------------------------

        // Header
        memcpy(tx_buffer + tx_len, frame_header, 3);
        tx_len += 3;

        // Epoch
        epoch_out = get_epoch();
        memcpy(tx_buffer + tx_len, &epoch_out, 4);
        tx_len += 4;

        // Output Opto
        memcpy(tx_buffer + tx_len, &opto_out, 1);
        tx_len += 1;

        // Output LED
        memcpy(tx_buffer + tx_len, &led_out, 2);
        tx_len += 2;

        // Buzzer
        memcpy(tx_buffer + tx_len + 0, &buzzer_waktu, 2);
        memcpy(tx_buffer + tx_len + 2, &buzzer_jumlah, 2);
        buzzer_waktu = 0;
        buzzer_jumlah = 0;
        tx_len += 4;

        // RS232 Baudrate
        for (int i = 0; i < 4; i++)
        {
            memcpy(tx_buffer + tx_len, &rs232[i].baudrate, 4);
            tx_len += 4;
        }

        // RS232 length
        for (int i = 0; i < 4; i++)
        {
            uint16_t len = rs232[i].tx_len = lwrb_get_full(&rs232[i].tx_ringbuffer);
            memcpy(tx_buffer + tx_len, &len, 2);
            tx_len += 2;
        }

        // RS232 Data
        for (int i = 0; i < 4; i++)
        {
            uint16_t len = rs232[i].tx_len;
            lwrb_read(&rs232[i].tx_ringbuffer, tx_buffer + tx_len, len);
            tx_len += len;
        }

        // Checksum
        uint8_t checksum = 0;
        for (int i = 0; i < tx_len; i++)
            checksum ^= tx_buffer[i];
        memcpy(tx_buffer + tx_len, &checksum, 1);
        tx_len += 1;

        //---------------------------------

        sp_nonblocking_write(serial_port, tx_buffer, tx_len);
    }
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void cllbck_sub_opto_out(const std_msgs::UInt8ConstPtr &msg)
{
    opto_out = msg->data;
}

void cllbck_sub_led_out(const std_msgs::UInt16ConstPtr &msg)
{
    led_out = msg->data;
}

void cllbck_sub_buzzer(const slff::buzzerConstPtr &msg)
{
    buzzer_waktu = msg->waktu;
    buzzer_jumlah = msg->jumlah;
}

void cllbck_sub_rs232_baudrate(const slff::rs232_baudrateConstPtr &msg)
{
    rs232[msg->channel - 1].baudrate = msg->baudrate;
}

void cllbck_sub_rs232_tx(const slff::rs232_dataConstPtr &msg)
{
    lwrb_write(&rs232[msg->channel - 1].tx_ringbuffer, msg->data.data(), msg->data.size());
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

int expansion_init()
{
    if (expansion_active)
    {
        for (int i = 0; i < 4; i++)
        {
            rs232[i].baudrate = 115200;

            lwrb_init(&rs232[i].tx_ringbuffer, rs232[i].tx_buffer, BUFFLEN);
            lwrb_init(&rs232[i].rx_ringbuffer, rs232[i].rx_buffer, BUFFLEN);
        }

        if (sp_get_port_by_name(realpath(expansion_port.c_str(), NULL), &serial_port) != SP_OK)
            return -1;
        sp_close(serial_port);
        if (sp_open(serial_port, (sp_mode)(SP_MODE_READ | SP_MODE_WRITE)) != SP_OK)
            return -1;
        if (sp_set_flowcontrol(serial_port, SP_FLOWCONTROL_NONE) != SP_OK)
            return -1;
        if (sp_set_baudrate(serial_port, expansion_baud) != SP_OK)
            return -1;
        if (sp_set_bits(serial_port, 8) != SP_OK)
            return -1;
        if (sp_set_parity(serial_port, SP_PARITY_NONE) != SP_OK)
            return -1;
        if (sp_set_stopbits(serial_port, 1) != SP_OK)
            return -1;
    }

    return 0;
}

int expansion_routine()
{
    if (expansion_active)
    {
        if (ros::Time::now().toSec() - serial_timer > 1)
        {
            serial_timer = ros::Time::now().toSec();
            rx_len = 0;
        }

        sp_return serial_return;
        while ((serial_return = sp_nonblocking_read(serial_port, &serial_data, 1)) > 0)
        {
            serial_timer = ros::Time::now().toSec();
            expansion_parser(serial_data);
        }

        if (serial_return < 0) return -1;
    }

    return 0;
}

void expansion_parser(uint8_t data)
{
    rx_buffer[rx_len] = data;

    //---------------------------------

    // Header
    if (rx_len >= 0 && rx_len < 3)
        if (rx_buffer[rx_len] == frame_header[rx_len])
            rx_len++;
        else
            rx_len = 0;
    // Epoch, Input Opto, RS232 Length, RS232 Data, dan Checksum
    else if (rx_len >= 3 && rx_len < 17 + rs232_rx_len)
        rx_len++;

    //---------------------------------

    // RS232 Length
    if (rx_len == 16)
    {
        // RS232 Length
        rs232_rx_len = 0;
        for (int i = 0; i < 4; i++)
        {
            uint16_t len = *(uint16_t *)(rx_buffer + 8 + 2 * i);
            rs232_rx_len += len;
        }
    }

    // Checksum, Epoch, Input Opto, dan RS232 Data
    if (rx_len == 17 + rs232_rx_len)
    {
        rx_len = 0;

        //-----------------------------

        // Checksum yang masuk
        uint8_t checksumA = *(uint8_t *)(rx_buffer + 16 + rs232_rx_len);

        // Checksum yang seharusnya
        uint8_t checksumB = 0;
        for (int i = 0; i < 16 + rs232_rx_len; i++)
            checksumB ^= rx_buffer[i];

        // Memeriksa checksum
        if (checksumA != checksumB)
            return;

        //-----------------------------

        // Epoch
        memcpy(&epoch_in, rx_buffer + 3, 4);

        // Input Opto
        memcpy(&opto_in, rx_buffer + 7, 1);
        // Publish
        std_msgs::UInt8 msg_opto_in;
        msg_opto_in.data = opto_in;
        pub_opto_in.publish(msg_opto_in);

        // RS232 Data
        rs232_rx_len = 0;
        for (int i = 0; i < 4; i++)
        {
            uint16_t len = *(uint16_t *)(rx_buffer + 8 + 2 * i);
            rs232_rx_len += len;

            lwrb_write(&rs232[i].rx_ringbuffer, rx_buffer + 16 + rs232_rx_len - len, len);
        }
        // Publish
        rs232_rx_len = 0;
        for (int i = 0; i < 4; i++)
        {
            uint16_t len = *(uint16_t *)(rx_buffer + 8 + 2 * i);
            rs232_rx_len += len;

            if (len > 0)
            {
                uint8_t data[len];
                lwrb_read(&rs232[i].rx_ringbuffer, data, len);

                slff::rs232_data msg_rs232_rx;
                msg_rs232_rx.channel = i + 1;
                msg_rs232_rx.data = std::vector<uint8_t>(data, data + len);
                pub_rs232_rx.publish(msg_rs232_rx);
            }
        }
    }
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

uint32_t get_epoch()
{
    time_t _time = time(NULL);
    struct tm _tm = *localtime(&_time);

    uint32_t epoch = mktime(&_tm) + _tm.tm_gmtoff;

    return epoch;
}