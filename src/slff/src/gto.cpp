#include "crc/crc.h"
#include "libserialport.h"
#include "ros/ros.h"
#include "slff/define.h"
#include "slff/gto_ack.h"
#include "slff/gto_init.h"
#include "slff/gto_notification.h"
#include "slff/gto_present.h"
#include "slff/gto_status_request.h"
#include "slff/gto_status_response.h"
#include "slff/gto_store.h"
#include "slff/misc.h"
#include "slff/rs232_baudrate.h"
#include "slff/rs232_data.h"

#define BUFFLEN 512

//=====Prototype
void cllbck_tim_100hz(const ros::TimerEvent &event);
void cllbck_tim_101hz(const ros::TimerEvent &event);

void cllbck_sub_gto_present(const slff::gto_presentConstPtr &msg);
void cllbck_sub_gto_notification(const slff::gto_notificationConstPtr &msg);
void cllbck_sub_gto_ack(const slff::gto_ackConstPtr &msg);
void cllbck_sub_gto_status_request(const slff::gto_status_requestConstPtr &msg);
void cllbck_sub_rs232_rx(const slff::rs232_dataConstPtr &msg);

int gto_init();
int gto_routine();
void gto_parser(uint8_t data);

//=====Parameter
bool gto_use_native;
std::string gto_port_native;
int gto_port;
int gto_baud;
int tipe_control_unit;
std::string tid;
std::string mid;
//=====Timer
ros::Timer tim_100hz;
ros::Timer tim_101hz;
//=====Subscriber
ros::Subscriber sub_gto_present;
ros::Subscriber sub_gto_notification;
ros::Subscriber sub_gto_ack;
ros::Subscriber sub_gto_status_request;
ros::Subscriber sub_rs232_rx;
//=====Publisher
ros::Publisher pub_gto_store;
ros::Publisher pub_gto_init;
ros::Publisher pub_gto_status_response;
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

//=====Frame
const uint8_t frame_header[3] = {'i', 'o', 't'};

int main(int argc, char **argv)
{
    ros::init(argc, argv, "gto");

    ros::NodeHandle NH;
    ros::MultiThreadedSpinner MTS;

    //=====Parameter
    NH.getParam("gto/use_native", gto_use_native);
    NH.getParam("gto/port_native", gto_port_native);
    NH.getParam("gto/port", gto_port);
    NH.getParam("gto/baud", gto_baud);
    NH.getParam("tipe_control_unit", tipe_control_unit);
    NH.getParam("tid", tid);
    NH.getParam("mid", mid);
    //=====Timer
    tim_100hz = NH.createTimer(ros::Duration(0.01), cllbck_tim_100hz);
    tim_101hz = NH.createTimer(ros::Duration(0.01), cllbck_tim_101hz);
    //=====Subscriber
    sub_gto_present = NH.subscribe("gto/present", 0, cllbck_sub_gto_present);
    sub_gto_notification = NH.subscribe("gto/notification", 0, cllbck_sub_gto_notification);
    sub_gto_ack = NH.subscribe("gto/ack", 0, cllbck_sub_gto_ack);
    sub_gto_status_request = NH.subscribe("gto/status_request", 0, cllbck_sub_gto_status_request);
    sub_rs232_rx = NH.subscribe("expansion/rs232_rx", 0, cllbck_sub_rs232_rx);
    //=====Publisher
    pub_gto_store = NH.advertise<slff::gto_store>("gto/store", 0);
    pub_gto_init = NH.advertise<slff::gto_init>("gto/init", 0);
    pub_gto_status_response = NH.advertise<slff::gto_status_response>("gto/status_response", 0);
    pub_rs232_baudrate = NH.advertise<slff::rs232_baudrate>("expansion/rs232_baudrate", 0);
    pub_rs232_tx = NH.advertise<slff::rs232_data>("expansion/rs232_tx", 0);
    //=====Help
    help.init(&NH);

    if (gto_init() == -1)
        ros::shutdown();

    MTS.spin();
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void cllbck_tim_100hz(const ros::TimerEvent &event)
{
    if (gto_routine() == -1)
        ros::shutdown();
}

void cllbck_tim_101hz(const ros::TimerEvent &event)
{
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void cllbck_sub_gto_present(const slff::gto_presentConstPtr &msg)
{
    tx_len = 0;

    //---------------------------------

    // Header
    memcpy(tx_buffer + tx_len, frame_header, 3);
    tx_len += 3;

    // Command
    tx_buffer[3] = 0x01;
    tx_len += 1;

    // Data
    if (tipe_control_unit == TIPE_OPEN ||
        tipe_control_unit == TIPE_EXIT ||
        tipe_control_unit == TIPE_EXIT_OPEN)
    {
        MEMCPY(tx_buffer + 4, &msg->no_seri_control_unit, 4);
        STRCPY(tx_buffer + 8, msg->rfid_tid, 24);
        MEMCPY(tx_buffer + 32, &msg->golongan_kendaraan, 1);
        MEMCPY(tx_buffer + 33, &msg->jenis_kendaraan, 1);
        STRCPY(tx_buffer + 34, msg->plat_no_rss, 16);
        STRCPY(tx_buffer + 50, msg->plat_no_anpr, 16);
        MEMCPY(tx_buffer + 66, &msg->no_gardu_entrance, 1);
        MEMCPY(tx_buffer + 67, &msg->no_gerbang_entrance, 1);
        MEMCPY(tx_buffer + 68, &msg->no_gardu_exit, 1);
        MEMCPY(tx_buffer + 69, &msg->no_gerbang_exit, 1);
        STRCPY(tx_buffer + 70, mid, 20);
        STRCPY(tx_buffer + 90, tid, 10);
        MEMCPY(tx_buffer + 100, &msg->saldo, 4);
        MEMCPY(tx_buffer + 104, &msg->tarif, 4);
        MEMCPY(tx_buffer + 108, &msg->entrance_day, 1);
        MEMCPY(tx_buffer + 109, &msg->entrance_month, 1);
        MEMCPY(tx_buffer + 110, &msg->entrance_year, 1);
        MEMCPY(tx_buffer + 111, &msg->entrance_hour, 1);
        MEMCPY(tx_buffer + 112, &msg->entrance_minute, 1);
        MEMCPY(tx_buffer + 113, &msg->entrance_second, 1);
        STRCPY(tx_buffer + 114, msg->hash, 64);
        tx_len += 174;
    }
    else if (tipe_control_unit == TIPE_ENTRANCE)
    {
        MEMCPY(tx_buffer + 4, &msg->no_seri_control_unit, 4);
        STRCPY(tx_buffer + 8, msg->rfid_tid, 24);
        MEMCPY(tx_buffer + 32, &msg->golongan_kendaraan, 1);
        MEMCPY(tx_buffer + 33, &msg->jenis_kendaraan, 1);
        STRCPY(tx_buffer + 34, msg->plat_no_rss, 16);
        STRCPY(tx_buffer + 50, msg->plat_no_anpr, 16);
        MEMCPY(tx_buffer + 66, &msg->no_gardu_entrance, 1);
        MEMCPY(tx_buffer + 67, &msg->no_gerbang_entrance, 1);
        STRCPY(tx_buffer + 68, mid, 20);
        STRCPY(tx_buffer + 88, tid, 10);
        MEMCPY(tx_buffer + 98, &msg->saldo, 4);
        tx_len += 98;
    }
    else if (tipe_control_unit == TIPE_OPEN_ENTRANCE)
    {
        MEMCPY(tx_buffer + 4, &msg->no_seri_control_unit, 4);
        STRCPY(tx_buffer + 8, msg->rfid_tid, 24);
        MEMCPY(tx_buffer + 32, &msg->golongan_kendaraan, 1);
        MEMCPY(tx_buffer + 33, &msg->jenis_kendaraan, 1);
        STRCPY(tx_buffer + 34, msg->plat_no_rss, 16);
        STRCPY(tx_buffer + 50, msg->plat_no_anpr, 16);
        MEMCPY(tx_buffer + 66, &msg->no_gardu_entrance, 1);
        MEMCPY(tx_buffer + 67, &msg->no_gerbang_entrance, 1);
        STRCPY(tx_buffer + 68, mid, 20);
        STRCPY(tx_buffer + 88, tid, 10);
        MEMCPY(tx_buffer + 98, &msg->saldo, 4);
        MEMCPY(tx_buffer + 102, &msg->tarif, 4);
        STRCPY(tx_buffer + 106, msg->hash, 64);
        tx_len += 166;
    }

    // Checksum
    uint32_t checksum = CRC::Calculate(tx_buffer, tx_len, CRC::CRC_32());
    memcpy(tx_buffer + tx_len, &checksum, 4);
    tx_len += 4;

    //---------------------------------

    if (gto_use_native)
        sp_nonblocking_write(serial_port, tx_buffer, tx_len);
    else if (!gto_use_native)
        help.rs232_tx(gto_port, std::vector<uint8_t>(tx_buffer, tx_buffer + tx_len));
}

void cllbck_sub_gto_notification(const slff::gto_notificationConstPtr &msg)
{
    tx_len = 0;

    //---------------------------------

    // Header
    memcpy(tx_buffer + tx_len, frame_header, 3);
    tx_len += 3;

    // Command
    tx_buffer[3] = 0x00;
    tx_len += 1;

    // Data
    MEMCPY(tx_buffer + 4, &msg->no_seri_control_unit, 4);
    STRCPY(tx_buffer + 8, msg->rfid_tid, 24);
    STRCPY(tx_buffer + 32, msg->message, 63);
    MEMCPY(tx_buffer + 95, &msg->golongan_kendaraan, 1);
    tx_len += 92;

    // Checksum
    uint32_t checksum = CRC::Calculate(tx_buffer, tx_len, CRC::CRC_32());
    memcpy(tx_buffer + tx_len, &checksum, 4);
    tx_len += 4;

    //---------------------------------

    if (gto_use_native)
        sp_nonblocking_write(serial_port, tx_buffer, tx_len);
    else if (!gto_use_native)
        help.rs232_tx(gto_port, std::vector<uint8_t>(tx_buffer, tx_buffer + tx_len));
}

void cllbck_sub_gto_ack(const slff::gto_ackConstPtr &msg)
{
    tx_len = 0;

    //---------------------------------

    // Header
    memcpy(tx_buffer + tx_len, frame_header, 3);
    tx_len += 3;

    // Command
    tx_buffer[3] = 0x02;
    tx_len += 1;

    // Data
    MEMCPY(tx_buffer + 4, &msg->status, 1);
    tx_len += 1;

    // Checksum
    uint32_t checksum = CRC::Calculate(tx_buffer, tx_len, CRC::CRC_32());
    memcpy(tx_buffer + tx_len, &checksum, 4);
    tx_len += 4;

    //---------------------------------

    if (gto_use_native)
        sp_nonblocking_write(serial_port, tx_buffer, tx_len);
    else if (!gto_use_native)
        help.rs232_tx(gto_port, std::vector<uint8_t>(tx_buffer, tx_buffer + tx_len));
}

void cllbck_sub_gto_status_request(const slff::gto_status_requestConstPtr &msg)
{
    tx_len = 0;

    //---------------------------------

    // Header
    memcpy(tx_buffer + tx_len, frame_header, 3);
    tx_len += 3;

    // Command
    tx_buffer[3] = 0x03;
    tx_len += 1;

    // Checksum
    uint32_t checksum = CRC::Calculate(tx_buffer, tx_len, CRC::CRC_32());
    memcpy(tx_buffer + tx_len, &checksum, 4);
    tx_len += 4;

    //---------------------------------

    if (gto_use_native)
        sp_nonblocking_write(serial_port, tx_buffer, tx_len);
    else if (!gto_use_native)
        help.rs232_tx(gto_port, std::vector<uint8_t>(tx_buffer, tx_buffer + tx_len));
}

void cllbck_sub_rs232_rx(const slff::rs232_dataConstPtr &msg)
{
    if (msg->channel == gto_port)
    {
        printf("RX = ");
        for (int i = 0; i < msg->data.size(); i++)
            printf("%02X ", msg->data[i]);
        printf("\n");

        for (int i = 0; i < msg->data.size(); i++)
            gto_parser(msg->data[i]);
    }
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

int gto_init()
{
    if (gto_use_native)
    {
        if (sp_get_port_by_name(realpath(gto_port_native.c_str(), NULL), &serial_port) != SP_OK)
            return -1;
        sp_close(serial_port);
        if (sp_open(serial_port, (sp_mode)(SP_MODE_READ | SP_MODE_WRITE)) != SP_OK)
            return -1;
        if (sp_set_flowcontrol(serial_port, SP_FLOWCONTROL_NONE) != SP_OK)
            return -1;
        if (sp_set_baudrate(serial_port, gto_baud) != SP_OK)
            return -1;
        if (sp_set_bits(serial_port, 8) != SP_OK)
            return -1;
        if (sp_set_parity(serial_port, SP_PARITY_NONE) != SP_OK)
            return -1;
        if (sp_set_stopbits(serial_port, 1) != SP_OK)
            return -1;
    }
    else if (!gto_use_native)
    {
        // Baudrate
        ros::Duration(2.5).sleep();
        help.rs232_baudrate(gto_port, gto_baud);
    }

    return 0;
}

int gto_routine()
{
    if (gto_use_native)
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
            gto_parser(serial_data);
        }

        if (serial_return < 0) return -1;
    }

    return 0;
}

void gto_parser(uint8_t data)
{
    static uint16_t frame_length;

    //---------------------------------

    rx_buffer[rx_len] = data;

    //---------------------------------

    // Header
    if (rx_len >= 0 && rx_len < 3)
        if (rx_buffer[rx_len] == frame_header[rx_len])
            rx_len++;
        else
            rx_len = 0;
    // Data dan Checksum
    else if (rx_len >= 3 && rx_len < 8 + frame_length)
        rx_len++;

    //---------------------------------

    // Frame Length
    if (rx_len == 4)
    {
        if (rx_buffer[3] == 0x00)
        {
            frame_length = 14;
        }
        else if (rx_buffer[3] == 0x01)
        {
            if (tipe_control_unit == TIPE_OPEN ||
                tipe_control_unit == TIPE_EXIT ||
                tipe_control_unit == TIPE_EXIT_OPEN)
                frame_length = 123;
            else if (tipe_control_unit == TIPE_ENTRANCE)
                frame_length = 53;
            else if (tipe_control_unit == TIPE_OPEN_ENTRANCE)
                frame_length = 117;
        }
        else if (rx_buffer[3] == 0x03)
        {
            frame_length = 1;
        }
    }

    // Data dan Checksum
    if (rx_len == 8 + frame_length)
    {
        rx_len = 0;

        //-----------------------------

        // Checksum yang masuk
        uint32_t checksumA = *(uint32_t *)(rx_buffer + 4 + frame_length);

        // Checksum yang seharusnya
        uint32_t checksumB = CRC::Calculate(rx_buffer, 4 + frame_length, CRC::CRC_32());

        // Memeriksa checksum
        if (checksumA != checksumB)
            return;

        //-----------------------------

        // Data
        if (rx_buffer[3] == 0x00)
        {
            slff::gto_init msg_gto_init;
            MEMCPY(&msg_gto_init.no_shift, rx_buffer + 4, 1);
            MEMCPY(&msg_gto_init.no_perioda, rx_buffer + 5, 1);
            MEMCPY(&msg_gto_init.no_resi, rx_buffer + 6, 4);
            MEMCPY(&msg_gto_init.no_kspt, rx_buffer + 10, 4);
            MEMCPY(&msg_gto_init.no_plt, rx_buffer + 14, 4);
            pub_gto_init.publish(msg_gto_init);
        }
        else if (rx_buffer[3] == 0x01)
        {
            slff::gto_store msg_gto_store;
            if (tipe_control_unit == TIPE_OPEN ||
                tipe_control_unit == TIPE_EXIT ||
                tipe_control_unit == TIPE_EXIT_OPEN)
            {
                MEMCPY(&msg_gto_store.no_seri_control_unit, rx_buffer + 4, 4);
                MEMCPY(&msg_gto_store.metode_pembayaran, rx_buffer + 8, 1);
                STRCPY2(msg_gto_store.rfid_tid, rx_buffer + 9, 24);
                MEMCPY(&msg_gto_store.entrance_day, rx_buffer + 33, 1);
                MEMCPY(&msg_gto_store.entrance_month, rx_buffer + 34, 1);
                MEMCPY(&msg_gto_store.entrance_year, rx_buffer + 35, 1);
                MEMCPY(&msg_gto_store.entrance_hour, rx_buffer + 36, 1);
                MEMCPY(&msg_gto_store.entrance_minute, rx_buffer + 37, 1);
                MEMCPY(&msg_gto_store.entrance_second, rx_buffer + 38, 1);
                MEMCPY(&msg_gto_store.exit_day, rx_buffer + 39, 1);
                MEMCPY(&msg_gto_store.exit_month, rx_buffer + 40, 1);
                MEMCPY(&msg_gto_store.exit_year, rx_buffer + 41, 1);
                MEMCPY(&msg_gto_store.exit_hour, rx_buffer + 42, 1);
                MEMCPY(&msg_gto_store.exit_minute, rx_buffer + 43, 1);
                MEMCPY(&msg_gto_store.exit_second, rx_buffer + 44, 1);
                MEMCPY(&msg_gto_store.report_day, rx_buffer + 45, 1);
                MEMCPY(&msg_gto_store.report_month, rx_buffer + 46, 1);
                MEMCPY(&msg_gto_store.report_year, rx_buffer + 47, 1);
                MEMCPY(&msg_gto_store.kode_ruas, rx_buffer + 48, 1);
                MEMCPY(&msg_gto_store.no_shift, rx_buffer + 49, 1);
                MEMCPY(&msg_gto_store.no_perioda, rx_buffer + 50, 1);
                MEMCPY(&msg_gto_store.no_resi, rx_buffer + 51, 4);
                MEMCPY(&msg_gto_store.no_kspt, rx_buffer + 55, 4);
                MEMCPY(&msg_gto_store.no_plt, rx_buffer + 59, 4);
                STRCPY2(msg_gto_store.hash, rx_buffer + 63, 64);
            }
            else if (tipe_control_unit == TIPE_ENTRANCE)
            {
                MEMCPY(&msg_gto_store.no_seri_control_unit, rx_buffer + 4, 4);
                MEMCPY(&msg_gto_store.metode_pembayaran, rx_buffer + 8, 1);
                STRCPY2(msg_gto_store.rfid_tid, rx_buffer + 9, 24);
                MEMCPY(&msg_gto_store.entrance_day, rx_buffer + 33, 1);
                MEMCPY(&msg_gto_store.entrance_month, rx_buffer + 34, 1);
                MEMCPY(&msg_gto_store.entrance_year, rx_buffer + 35, 1);
                MEMCPY(&msg_gto_store.entrance_hour, rx_buffer + 36, 1);
                MEMCPY(&msg_gto_store.entrance_minute, rx_buffer + 37, 1);
                MEMCPY(&msg_gto_store.entrance_second, rx_buffer + 38, 1);
                MEMCPY(&msg_gto_store.report_day, rx_buffer + 39, 1);
                MEMCPY(&msg_gto_store.report_month, rx_buffer + 40, 1);
                MEMCPY(&msg_gto_store.report_year, rx_buffer + 41, 1);
                MEMCPY(&msg_gto_store.kode_ruas, rx_buffer + 42, 1);
                MEMCPY(&msg_gto_store.no_shift, rx_buffer + 43, 1);
                MEMCPY(&msg_gto_store.no_perioda, rx_buffer + 44, 1);
                MEMCPY(&msg_gto_store.no_resi, rx_buffer + 45, 4);
                MEMCPY(&msg_gto_store.no_kspt, rx_buffer + 49, 4);
                MEMCPY(&msg_gto_store.no_plt, rx_buffer + 53, 4);
            }
            else if (tipe_control_unit == TIPE_OPEN_ENTRANCE)
            {
                MEMCPY(&msg_gto_store.no_seri_control_unit, rx_buffer + 4, 4);
                MEMCPY(&msg_gto_store.metode_pembayaran, rx_buffer + 8, 1);
                STRCPY2(msg_gto_store.rfid_tid, rx_buffer + 9, 24);
                MEMCPY(&msg_gto_store.entrance_day, rx_buffer + 33, 1);
                MEMCPY(&msg_gto_store.entrance_month, rx_buffer + 34, 1);
                MEMCPY(&msg_gto_store.entrance_year, rx_buffer + 35, 1);
                MEMCPY(&msg_gto_store.entrance_hour, rx_buffer + 36, 1);
                MEMCPY(&msg_gto_store.entrance_minute, rx_buffer + 37, 1);
                MEMCPY(&msg_gto_store.entrance_second, rx_buffer + 38, 1);
                MEMCPY(&msg_gto_store.report_day, rx_buffer + 39, 1);
                MEMCPY(&msg_gto_store.report_month, rx_buffer + 40, 1);
                MEMCPY(&msg_gto_store.report_year, rx_buffer + 41, 1);
                MEMCPY(&msg_gto_store.no_shift, rx_buffer + 43, 1);
                MEMCPY(&msg_gto_store.no_perioda, rx_buffer + 44, 1);
                MEMCPY(&msg_gto_store.no_resi, rx_buffer + 45, 4);
                MEMCPY(&msg_gto_store.no_kspt, rx_buffer + 49, 4);
                MEMCPY(&msg_gto_store.no_plt, rx_buffer + 53, 4);
                STRCPY2(msg_gto_store.hash, rx_buffer + 57, 64);
            }
            pub_gto_store.publish(msg_gto_store);
        }
        else if (rx_buffer[3] == 0x03)
        {
            slff::gto_status_response msg_gto_status_response;
            MEMCPY(&msg_gto_status_response.status, rx_buffer + 4, 1);
            pub_gto_status_response.publish(msg_gto_status_response);
        }
    }
}