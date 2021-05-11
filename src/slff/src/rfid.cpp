#include "libserialport.h"
#include "ros/ros.h"
#include "slff/define.h"
#include "slff/misc.h"
#include "slff/rfid_ack.h"
#include "slff/rfid_status_request.h"
#include "slff/rfid_status_response.h"
#include "slff/rfid_tag.h"
#include "slff/rs232_baudrate.h"
#include "slff/rs232_data.h"

#define BUFFLEN 512

//=====Prototype
void cllbck_tim_50hz(const ros::TimerEvent &event);
void cllbck_tim_51hz(const ros::TimerEvent &event);

void cllbck_sub_rfid_ack(const slff::rfid_ackConstPtr &msg);
void cllbck_sub_rfid_status_request(const slff::rfid_status_requestConstPtr &msg);
void cllbck_sub_rs232_rx(const slff::rs232_dataConstPtr &msg);

int rfid_init();
int rfid_routine();
void rfid_parser(uint8_t data);

void invengo_init();
void invengo_parser(uint8_t data);
void cu1_init();
void cu1_parser(uint8_t data);

void rfid_entry(std::vector<uint8_t> epc, std::vector<uint8_t> tid, std::vector<uint8_t> userdata);

//=====Parameter
bool rfid_use_native;
std::string rfid_port_native;
int rfid_port;
int rfid_baud;
int rfid_type;
int rfid_power;
//=====Timer
ros::Timer tim_50hz;
ros::Timer tim_51hz;
//=====Subscriber
ros::Subscriber sub_rfid_ack;
ros::Subscriber sub_rfid_status_request;
ros::Subscriber sub_rs232_rx;
//=====Publisher
ros::Publisher pub_rfid_tag;
ros::Publisher pub_rfid_status_response;
ros::Publisher pub_rs232_baudrate;
ros::Publisher pub_rs232_tx;
//=====Help
misc help;

//=====Serial Connection
struct sp_port *serial_port;
double serial_timer;
uint8_t serial_data;
uint8_t tx_buffer[BUFFLEN];
uint8_t rx_buffer[BUFFLEN];
uint16_t tx_len;
uint16_t rx_len;

//=====Frame Invengo
// Start reader dengan parameter 12 byte EPC, 12 byte TID, dan 16 byte userdata
const uint8_t invengo_start1[] =
    {0x55, 0x00, 0x03, 0x62, 0x69, 0x00, 0x4D, 0xAE};
const uint8_t invengo_start2[] =
    {0x55, 0x00, 0x07, 0x97, 0x81, 0x00, 0x01, 0x06, 0x00, 0x08, 0x02, 0xA1};
// Stop reader untuk mengubah parameter saat inisialisasi
const uint8_t invengo_stop[] =
    {0x55, 0x00, 0x01, 0x61, 0x07, 0x46};
// Single tag mode agar pembacaan bisa sangat cepat
const uint8_t invengo_config_mode[] =
    {0x55, 0x00, 0x04, 0x63, 0x6D, 0x01, 0x02, 0x3C, 0x99};
// 0 interval, 0 misreading filter agar pembacaan bisa sangat cepat
const uint8_t invengo_config_time[] =
    {0x55, 0x00, 0x04, 0x8F, 0x02, 0x00, 0x00, 0x4D, 0xF7};
// Konfigurasi power dari 11dBm sampai 30dBm
const uint8_t invengo_config_power[][10] =
    {{0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x0B, 0x4D, 0x8E},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x0C, 0xCD, 0x9F},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x0D, 0x4D, 0x9A},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x0E, 0x4D, 0x90},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x0F, 0xCD, 0x95},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x10, 0x4D, 0xD4},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x11, 0xCD, 0xD1},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x12, 0xCD, 0xDB},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x13, 0x4D, 0xDE},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x14, 0xCD, 0xCF},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x15, 0x4D, 0xCA},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x16, 0x4D, 0xC0},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x17, 0xCD, 0xC5},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x18, 0xCD, 0xE7},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x19, 0x4D, 0xE2},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x1A, 0x4D, 0xE8},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x1B, 0xCD, 0xED},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x1C, 0x4D, 0xFC},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x1D, 0xCD, 0xF9},
     {0x55, 0x00, 0x05, 0x63, 0x65, 0x02, 0x00, 0x1E, 0xCD, 0xF3}};
// Connect/Disconnect request ke reader
const uint8_t invengo_request[] =
    {0x55, 0x00, 0x02, 0xD2, 0x00, 0xEC, 0x24};
// Connect/Disconnect response dari reader
const uint8_t invengo_response[] =
    {0x55, 0x00, 0x04, 0xD2, 0x00, 0x00, 0x00, 0x69, 0xC4};

//=====Frame CU1
const uint8_t cu1_start[] =
    {0x10, 0x20};
const uint8_t cu1_stop[] =
    {0x10, 0x03};
const uint8_t cu1_header_tx[] =
    {0x08, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00};
const uint8_t cu1_header_rx[] =
    {0x08, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00};

//=====RFID
typedef struct
{
    std::vector<uint8_t> epc;
    std::vector<uint8_t> tid;
    std::vector<uint8_t> userdata;
    double time;
} rfid;

double last_rfid_time;

int main(int argc, char **argv)
{
    ros::init(argc, argv, "rfid");

    ros::NodeHandle NH;
    ros::MultiThreadedSpinner MTS;

    //=====Parameter
    NH.getParam("rfid/use_native", rfid_use_native);
    NH.getParam("rfid/port_native", rfid_port_native);
    NH.getParam("rfid/port", rfid_port);
    NH.getParam("rfid/baud", rfid_baud);
    NH.getParam("rfid/type", rfid_type);
    NH.getParam("rfid/power", rfid_power);
    //=====Timer
    tim_50hz = NH.createTimer(ros::Duration(0.02), cllbck_tim_50hz);
    tim_51hz = NH.createTimer(ros::Duration(0.02), cllbck_tim_51hz);
    //=====Subscriber
    sub_rfid_ack = NH.subscribe("rfid/ack", 0, cllbck_sub_rfid_ack);
    sub_rfid_status_request = NH.subscribe("rfid/status_request", 0, cllbck_sub_rfid_status_request);
    sub_rs232_rx = NH.subscribe("expansion/rs232_rx", 0, cllbck_sub_rs232_rx);
    //=====Publisher
    pub_rfid_tag = NH.advertise<slff::rfid_tag>("rfid/tag", 0);
    pub_rfid_status_response = NH.advertise<slff::rfid_status_response>("rfid/status_response", 0);
    pub_rs232_baudrate = NH.advertise<slff::rs232_baudrate>("expansion/rs232_baudrate", 0);
    pub_rs232_tx = NH.advertise<slff::rs232_data>("expansion/rs232_tx", 0);
    //=====Help
    help.init(&NH);

    if (rfid_init() == -1)
        ros::shutdown();

    MTS.spin();
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void cllbck_tim_50hz(const ros::TimerEvent &event)
{
    if (rfid_routine() == -1)
        ros::shutdown();
}

void cllbck_tim_51hz(const ros::TimerEvent &event)
{
    // // Inisialisasi ulang rfid jika idle selama 60 detik
    // if (ros::Time::now().toSec() - last_rfid_time > 60)
    // {
    //     last_rfid_time = ros::Time::now().toSec();

    //     if (rfid_type == RFID_INVENGO)
    //         invengo_init();
    //     else if (rfid_type == RFID_CU1)
    //         cu1_init();
    // }
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void cllbck_sub_rfid_ack(const slff::rfid_ackConstPtr &msg)
{
}

void cllbck_sub_rfid_status_request(const slff::rfid_status_requestConstPtr &msg)
{
    if (rfid_use_native)
    {
        if (rfid_type == RFID_INVENGO)
            sp_nonblocking_write(serial_port, invengo_request, sizeof(invengo_request));
    }
    else if (!rfid_use_native)
    {
        if (rfid_type == RFID_INVENGO)
            help.rs232_tx(rfid_port, std::vector<uint8_t>(invengo_request, invengo_request + sizeof(invengo_request)));
    }
}

void cllbck_sub_rs232_rx(const slff::rs232_dataConstPtr &msg)
{
    if (msg->channel == rfid_port)
        for (int i = 0; i < msg->data.size(); i++)
            rfid_parser(msg->data[i]);
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

int rfid_init()
{
    last_rfid_time = ros::Time::now().toSec();

    if (rfid_use_native)
    {
        if (sp_get_port_by_name(realpath(rfid_port_native.c_str(), NULL), &serial_port) != SP_OK)
            return -1;
        sp_close(serial_port);
        if (sp_open(serial_port, (sp_mode)(SP_MODE_READ | SP_MODE_WRITE)) != SP_OK)
            return -1;
        if (sp_set_flowcontrol(serial_port, SP_FLOWCONTROL_NONE) != SP_OK)
            return -1;
        if (sp_set_baudrate(serial_port, rfid_baud) != SP_OK)
            return -1;
        if (sp_set_bits(serial_port, 8) != SP_OK)
            return -1;
        if (sp_set_parity(serial_port, SP_PARITY_NONE) != SP_OK)
            return -1;
        if (sp_set_stopbits(serial_port, 1) != SP_OK)
            return -1;
    }
    else if (!rfid_use_native)
    {
        // Baudrate
        ros::Duration(2.5).sleep();
        help.rs232_baudrate(rfid_port, rfid_baud);
    }

    if (rfid_type == RFID_INVENGO)
        invengo_init();
    else if (rfid_type == RFID_CU1)
        cu1_init();

    return 0;
}

int rfid_routine()
{
    if (rfid_use_native)
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
            rfid_parser(serial_data);
        }

        if (serial_return < 0) return -1;
    }

    return 0;
}

void rfid_parser(uint8_t data)
{
    if (rfid_type == RFID_INVENGO)
        invengo_parser(data);
    else if (rfid_type == RFID_CU1)
        cu1_parser(data);
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void invengo_init()
{
    if (rfid_power < 11)
        rfid_power = 0;
    else if (rfid_power > 30)
        rfid_power = 19;
    else
        rfid_power -= 11;

    if (rfid_use_native)
    {
        // Stop
        sp_nonblocking_write(serial_port, invengo_stop, sizeof(invengo_stop));
        // Config mode
        sp_nonblocking_write(serial_port, invengo_config_mode, sizeof(invengo_config_mode));
        // Config time
        sp_nonblocking_write(serial_port, invengo_config_time, sizeof(invengo_config_time));
        // Config power
        sp_nonblocking_write(serial_port, invengo_config_power[rfid_power], sizeof(invengo_config_power[rfid_power]));
        // Start
        sp_nonblocking_write(serial_port, invengo_start1, sizeof(invengo_start1));
        sp_nonblocking_write(serial_port, invengo_start2, sizeof(invengo_start2));
    }
    else if (!rfid_use_native)
    {
        // Stop
        help.rs232_tx(rfid_port, std::vector<uint8_t>(invengo_stop, invengo_stop + sizeof(invengo_stop)));
        // Config mode
        help.rs232_tx(rfid_port, std::vector<uint8_t>(invengo_config_mode, invengo_config_mode + sizeof(invengo_config_mode)));
        // Config time
        help.rs232_tx(rfid_port, std::vector<uint8_t>(invengo_config_time, invengo_config_time + sizeof(invengo_config_time)));
        // Config power
        help.rs232_tx(rfid_port, std::vector<uint8_t>(invengo_config_power[rfid_power], invengo_config_power[rfid_power] + sizeof(invengo_config_power[rfid_power])));
        // Start
        help.rs232_tx(rfid_port, std::vector<uint8_t>(invengo_start1, invengo_start1 + sizeof(invengo_start1)));
        help.rs232_tx(rfid_port, std::vector<uint8_t>(invengo_start2, invengo_start2 + sizeof(invengo_start2)));
    }
}

void invengo_parser(uint8_t data)
{
    static uint8_t is56 = 0;

    static uint8_t epc_length = 0;
    static uint8_t tid_length = 0;
    static uint8_t userdata_length = 0;

    //---------------------------------

    if (data == 0x55)
    {
        rx_len = 0;
        rx_buffer[rx_len++] = data;
    }
    else
    {
        if (is56 == 0 && data != 0x56)
            rx_buffer[rx_len++] = data;
        else if (is56 == 1 && data == 0x56)
            rx_buffer[rx_len++] = 0x55;
        else if (is56 == 1 && data == 0x57)
            rx_buffer[rx_len++] = 0x56;

        if (is56 == 0 && data == 0x56)
            is56 = 1;
        else
            is56 = 0;
    }

    //---------------------------------

    if (rx_buffer[3] == 0x97)
    {
        // EPC Length
        if (rx_len == 7)
            epc_length = rx_buffer[6];

        // TID Length
        if (rx_len == 8 + epc_length)
            tid_length = rx_buffer[7 + epc_length];

        // UserData Length
        if (rx_len == 9 + epc_length + tid_length)
            userdata_length = rx_buffer[8 + epc_length + tid_length];

        // EPC, TID, dan UserData
        if (rx_len == 9 + epc_length + tid_length + userdata_length)
        {
            std::vector<uint8_t> epc(
                rx_buffer + 7,
                rx_buffer + 7 + epc_length);
            std::vector<uint8_t> tid(
                rx_buffer + 8 + epc_length,
                rx_buffer + 8 + epc_length + tid_length);
            std::vector<uint8_t> userdata(
                rx_buffer + 9 + epc_length + tid_length,
                rx_buffer + 9 + epc_length + tid_length + userdata_length);

            rfid_entry(epc, tid, userdata);
        }
    }
    else if (rx_buffer[3] == 0xD2)
    {
        // Response
        if (rx_len == 4)
        {
            slff::rfid_status_response msg_rfid_status_response;
            msg_rfid_status_response.status = 0;
            pub_rfid_status_response.publish(msg_rfid_status_response);
        }
    }
}

void cu1_init()
{
}

void cu1_parser(uint8_t data)
{
    static uint16_t frame_length;
    static uint8_t frame_command;
    static uint8_t *frame_data;

    //---------------------------------

    rx_buffer[rx_len] = data;

    //---------------------------------

    // Start
    if (rx_len >= 0 && rx_len < 2)
        if (rx_buffer[rx_len] == cu1_start[rx_len - 0])
            rx_len++;
        else
            rx_len = 0;
    // Header RX
    else if (rx_len >= 2 && rx_len < 9)
        if (rx_buffer[rx_len] == cu1_header_rx[rx_len - 2])
            rx_len++;
        else
            rx_len = 0;
    // Frame length, frame data, dan checksum
    else if (rx_len >= 9 && rx_len < 12 + frame_length)
        rx_len++;
    // Stop
    else if (rx_len >= 12 + frame_length && rx_len < 14 + frame_length)
        if (rx_buffer[rx_len] == cu1_stop[rx_len - 12 - frame_length])
            rx_len++;
        else
            rx_len = 0;

    //---------------------------------

    // Frame length
    if (rx_len == 11)
    {
        frame_length = 0;
        frame_length += rx_buffer[9] << 8;
        frame_length += rx_buffer[10] << 0;
    }

    // Frame data dan checksum
    if (rx_len == 14 + frame_length)
    {
        rx_len = 0;

        //---------------------------------

        // Checksum yang masuk
        uint8_t checksumA = *(uint8_t *)(rx_buffer + 11 + frame_length);

        // Checksum yang seharusnya
        uint8_t checksumB = 0;
        for (int i = 2; i < 11 + frame_length; i++)
            checksumB ^= rx_buffer[i];

        // Memeriksa checksum
        if (checksumA != checksumB)
            return;

        //---------------------------------

        // Frame data dan frame command
        frame_data = rx_buffer + 11;
        frame_command = rx_buffer[11];

        //---------------------------------

        // RFID
        if (frame_command == 0x40)
        {
            std::string raw_epc = boost::algorithm::unhex(std::string(frame_data + 25, frame_data + 25 + 24));
            std::string raw_tid = boost::algorithm::unhex(std::string(frame_data + 1, frame_data + 1 + 24));
            std::vector<uint8_t> epc(raw_epc.data(), raw_epc.data() + 12);
            std::vector<uint8_t> tid(raw_tid.data(), raw_tid.data() + 12);
            std::vector<uint8_t> userdata;

            rfid_entry(epc, tid, userdata);
        }
        // Response
        else if (frame_command == 0x41)
        {
            slff::rfid_status_response msg_rfid_status_response;
            msg_rfid_status_response.status = 0;
            pub_rfid_status_response.publish(msg_rfid_status_response);
        }

        //---------------------------------

        // Start
        for (int i = 0; i < 2; i++)
            tx_buffer[i] = cu1_start[i - 0];
        // Header TX
        for (int i = 2; i < 9; i++)
            tx_buffer[i] = cu1_header_tx[i - 2];
        // Frame Length
        tx_buffer[9] = (5 >> 8) & 0xff;
        tx_buffer[10] = (5 >> 0) & 0xff;
        // Frame data
        tx_buffer[11] = frame_command;
        for (int i = 12; i < 16; i++)
            tx_buffer[i] = '0';
        // Checksum
        tx_buffer[11 + 5] = 0;
        for (int i = 2; i < 11 + 5; i++)
            tx_buffer[11 + 5] ^= tx_buffer[i];
        // Stop
        for (int i = 12 + 5; i < 14 + 5; i++)
            tx_buffer[i] = cu1_stop[i - 12 - 5];

        if (rfid_use_native)
        {
            sp_nonblocking_write(serial_port, tx_buffer, 14 + 5);
        }
        else if (!rfid_use_native)
        {
            help.rs232_tx(rfid_port, std::vector<uint8_t>(tx_buffer, tx_buffer + 14 + 5));
        }
    }
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void rfid_entry(std::vector<uint8_t> epc, std::vector<uint8_t> tid, std::vector<uint8_t> userdata)
{
    static std::vector<rfid> rfid_pool;

    // Safety jika panjang EPC dan TID kurang dari 12 byte
    if (epc.size() < 12 || tid.size() < 12)
        return;

    uint8_t isNew = 1;

    // Menghapus rfid yang umurnya lebih dari 60 detik
    for (int i = 0; i < rfid_pool.size(); i++)
        if (ros::Time::now().toSec() - rfid_pool[i].time > 60)
        {
            rfid_pool.erase(rfid_pool.begin() + i--);
            isNew = 1;
        }

    // Memperbarui rfid yang umurnya kurang dari 60 detik
    for (int i = 0; i < rfid_pool.size(); i++)
        if (rfid_pool[i].tid == tid)
        {
            rfid_pool[i].time = ros::Time::now().toSec();
            isNew = 0;
        }

    if (isNew)
    {
        rfid rfid_pool_buffer;
        rfid_pool_buffer.epc = epc;
        rfid_pool_buffer.tid = tid;
        rfid_pool_buffer.userdata = userdata;
        rfid_pool_buffer.time = ros::Time::now().toSec();
        rfid_pool.push_back(rfid_pool_buffer);

        slff::rfid_tag msg_rfid_tag;
        msg_rfid_tag.epc = epc;
        msg_rfid_tag.tid = tid;
        msg_rfid_tag.userdata = userdata;
        pub_rfid_tag.publish(msg_rfid_tag);
    }

    // Menandai waktu terakhir rfid berhasil ter-scan
    last_rfid_time = ros::Time::now().toSec();
}