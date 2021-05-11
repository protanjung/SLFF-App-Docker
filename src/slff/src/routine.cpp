#include "boost/filesystem.hpp"
#include "boost/thread/mutex.hpp"
#include "ros/ros.h"
#include "slff/define.h"
#include "slff/exporter_peripheral_status.h"
#include "slff/exporter_version.h"
#include "slff/gto_ack.h"
#include "slff/gto_init.h"
#include "slff/gto_notification.h"
#include "slff/gto_present.h"
#include "slff/gto_status_request.h"
#include "slff/gto_status_response.h"
#include "slff/gto_store.h"
#include "slff/misc.h"
#include "slff/rfid_ack.h"
#include "slff/rfid_status_request.h"
#include "slff/rfid_status_response.h"
#include "slff/rfid_tag.h"
#include "slff/rss_check.h"
#include "slff/rss_store.h"
#include "slff/version.h"
#include "json/json.hpp"
#include <deque>

//=====Prototype
void cllbck_tim_50hz(const ros::TimerEvent &event);
void cllbck_tim_51hz(const ros::TimerEvent &event);
void cllbck_tim_52hz(const ros::TimerEvent &event);
void cllbck_tim_53hz(const ros::TimerEvent &event);
void cllbck_tim_1hz(const ros::TimerEvent &event);

void cllbck_sub_rfid_tag(const slff::rfid_tagConstPtr &msg);
void cllbck_sub_gto_store(const slff::gto_storeConstPtr &msg);
void cllbck_sub_gto_init(const slff::gto_initConstPtr &msg);
void cllbck_sub_gto_status_response(const slff::gto_status_responseConstPtr &msg);
void cllbck_sub_rfid_status_response(const slff::rfid_status_responseConstPtr &msg);

int routine_init();
int routine_routine();

bool help_rss_check(uint8_t n);
bool help_rss_store(uint8_t n);
void help_gto_present(uint8_t n);
void help_gto_notification(uint8_t n);
void help_gto_store(uint8_t n, slff::gto_store msg);
void help_gto_init(slff::gto_init msg);

void report_rss_check(uint8_t n, slff::rss_check srv);
void report_rss_store(uint8_t n, slff::rss_store srv);
void report_gto_present(slff::gto_present msg);
void report_gto_notification(slff::gto_notification msg);
void report_gto_store(slff::gto_store msg);
void report_gto_init(slff::gto_init msg);
void report_time(uint8_t n);

void help_gto_ack_ok();
void help_gto_ack_error();
void help_rfid_ack_ok();
void help_rfid_ack_error();
void help_cdp(const char *message);

void queue_entry();

std::string rfid_encrypt(std::string input);
std::string rfid_decrypt(std::string input);

//=====Parameter
int tipe_control_unit;
std::string tid;
std::string mid;
int no_gardu;
int no_gerbang;
bool auth_rfid;
double gto_status_interval;
double rfid_status_interval;
//=====Timer
ros::Timer tim_50hz;
ros::Timer tim_51hz;
ros::Timer tim_52hz;
ros::Timer tim_53hz;
ros::Timer tim_1hz;
//=====Subscriber
ros::Subscriber sub_rfid_tag;
ros::Subscriber sub_gto_store;
ros::Subscriber sub_gto_init;
ros::Subscriber sub_gto_status_response;
ros::Subscriber sub_rfid_status_response;
//=====Publisher
ros::Publisher pub_rfid_ack;
ros::Publisher pub_gto_present;
ros::Publisher pub_gto_notification;
ros::Publisher pub_gto_ack;
ros::Publisher pub_gto_status_request;
ros::Publisher pub_rfid_status_request;
ros::Publisher pub_exporter_peripheral_status;
ros::Publisher pub_exporter_version;
//=====ServiceClient
ros::ServiceClient cli_rss_check;
ros::ServiceClient cli_rss_store;
//=====Mutex
boost::mutex mutex_kr;
//=====Help
misc help;

//=====JSON
std::string slff_json_path;
nlohmann::json slff_json;

//=====RFID
std::string rfid_epc;
std::string rfid_tid;
std::string rfid_userdata;

typedef struct
{
    std::string tid;
    double time;
} rfid;

std::vector<rfid> rfid_present_pool;

//=====Kendaraan
typedef struct
{
    uint8_t algorithm_state = 0;

    double time_detect;
    double time_rss_check;
    double time_gto_present;
    double time_gto_store;
    double time_rss_store;

    uint8_t rss_check_retry = 0;
    uint8_t rss_store_retry = 0;
    uint8_t gto_present_retry = 0;
    uint8_t gto_notification_retry = 0;
    double gto_present_timer = 0;
    double gto_notification_timer = 0;

    uint32_t no_seri_control_unit = 0;

    uint8_t metode_pembayaran = 0;

    uint8_t rfid_status = 0;
    uint8_t rfid_flag = 0;
    std::string rfid_epc = "";
    std::string rfid_tid = "";
    std::string rfid_userdata = "";

    uint8_t golongan_kendaraan = 0;
    uint8_t jenis_kendaraan = 0;
    std::string plat_no_rss = "";
    std::string plat_no_anpr = "";

    uint32_t saldo = 0;
    uint32_t tarif = 0;

    uint8_t no_gardu_entrance = 0;
    uint8_t no_gerbang_entrance = 0;
    uint8_t no_gardu_exit = 0;
    uint8_t no_gerbang_exit = 0;
    uint8_t entrance_year = 0;
    uint8_t entrance_month = 0;
    uint8_t entrance_day = 0;
    uint8_t entrance_hour = 0;
    uint8_t entrance_minute = 0;
    uint8_t entrance_second = 0;
    uint8_t exit_year = 0;
    uint8_t exit_month = 0;
    uint8_t exit_day = 0;
    uint8_t exit_hour = 0;
    uint8_t exit_minute = 0;
    uint8_t exit_second = 0;

    uint8_t transaction_year = 0;
    uint8_t transaction_month = 0;
    uint8_t transaction_day = 0;
    uint8_t transaction_hour = 0;
    uint8_t transaction_minute = 0;
    uint8_t transaction_second = 0;

    uint8_t report_year = 0;
    uint8_t report_month = 0;
    uint8_t report_day = 0;

    uint8_t kode_ruas = 0;

    uint8_t no_shift = 0;
    uint8_t no_perioda = 0;
    uint32_t no_resi = 0;
    uint32_t no_kspt = 0;
    uint32_t no_plt = 0;
    std::string hash_rss = "";
    std::string hash_gto = "";
} vehicle;

std::deque<vehicle> kr;

//=====Algoritma
int no_seri_control_unit = 1;

//=====Peripheral Status
typedef struct
{
    uint8_t status = 255;
    double datetime = 0;
} periph_status;

periph_status gto_status;
periph_status rfid_status;
double gto_status_timer;
double rfid_status_timer;

int main(int argc, char **argv)
{
    ros::init(argc, argv, "routine");

    ros::NodeHandle NH;
    ros::MultiThreadedSpinner MTS;

    //=====Parameter
    NH.getParam("tipe_control_unit", tipe_control_unit);
    NH.getParam("tid", tid);
    NH.getParam("mid", mid);
    NH.getParam("no_gardu", no_gardu);
    NH.getParam("no_gerbang", no_gerbang);
    NH.getParam("auth_rfid", auth_rfid);
    NH.getParam("gto_status_interval", gto_status_interval);
    NH.getParam("rfid_status_interval", rfid_status_interval);
    //=====Timer
    tim_50hz = NH.createTimer(ros::Duration(0.02), cllbck_tim_50hz);
    tim_51hz = NH.createTimer(ros::Duration(0.02), cllbck_tim_51hz);
    tim_52hz = NH.createTimer(ros::Duration(0.02), cllbck_tim_52hz);
    tim_53hz = NH.createTimer(ros::Duration(0.02), cllbck_tim_53hz);
    tim_1hz = NH.createTimer(ros::Duration(1), cllbck_tim_1hz);
    //=====Subscriber
    sub_rfid_tag = NH.subscribe("rfid/tag", 0, cllbck_sub_rfid_tag);
    sub_gto_store = NH.subscribe("gto/store", 0, cllbck_sub_gto_store);
    sub_gto_init = NH.subscribe("gto/init", 9, cllbck_sub_gto_init);
    sub_gto_status_response = NH.subscribe("gto/status_response", 0, cllbck_sub_gto_status_response);
    sub_rfid_status_response = NH.subscribe("rfid/status_response", 0, cllbck_sub_rfid_status_response);
    //=====Publisher
    pub_rfid_ack = NH.advertise<slff::rfid_ack>("rfid/ack", 0);
    pub_gto_present = NH.advertise<slff::gto_present>("gto/present", 0);
    pub_gto_notification = NH.advertise<slff::gto_notification>("gto/notification", 0);
    pub_gto_ack = NH.advertise<slff::gto_ack>("gto/ack", 0);
    pub_gto_status_request = NH.advertise<slff::gto_status_request>("gto/status_request", 0);
    pub_rfid_status_request = NH.advertise<slff::rfid_status_request>("rfid/status_request", 0);
    pub_exporter_peripheral_status = NH.advertise<slff::exporter_peripheral_status>("exporter/peripheral_status", 0);
    pub_exporter_version = NH.advertise<slff::exporter_version>("exporter/version", 0);
    //=====ServiceClient
    cli_rss_check = NH.serviceClient<slff::rss_check>("rss/check");
    cli_rss_store = NH.serviceClient<slff::rss_store>("rss/store");
    //=====Help
    help.init(&NH);

    if (routine_init() == -1)
        ros::shutdown();

    MTS.spin();
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

// Rutin RSS_CHECK
void cllbck_tim_50hz(const ros::TimerEvent &event)
{
    if (kr.size() == 0)
        return;

    tim_50hz.stop();
    mutex_kr.lock();

    int i;

    // Mengubah state IDDLE ke RSS_CHECK
    //----------------------------------
    for (i = 0; i < kr.size(); i++)
    {
        if (kr[i].algorithm_state == STATE_IDDLE)
            kr[i].algorithm_state = STATE_RSS_CHECK;
        if (kr[i].algorithm_state == STATE_RSS_CHECK)
            break;
    }

    if (i != kr.size())
    {
        // Jika kendaraan memiliki RFID
        //-----------------------------
        if (kr[i].algorithm_state == STATE_RSS_CHECK)
        {
            // Jika berhasil
            if (help_rss_check(i))
            {
                // Mengubah state RSS_CHECK ke GTO_PRESENT
                kr[i].algorithm_state = STATE_GTO_PRESENT;
            }
            // Jika gagal
            else
            {
                if (++kr[i].rss_check_retry == 5)
                    // Mengubah state RSS_CHECK ke GTO_PRESENT
                    kr[i].algorithm_state = STATE_GTO_PRESENT;
            }
        }
    }

    mutex_kr.unlock();
    tim_50hz.start();
}

// Rutin GTO_PRESENT dan GTO_NOTIFICATION
void cllbck_tim_51hz(const ros::TimerEvent &event)
{
    if (kr.size() == 0)
        return;

    tim_51hz.stop();
    mutex_kr.lock();

    int i;

    // Mengubah state GTO_PRESENT_DONE ke GTO_PRESENT
    //-----------------------------------------------
    for (i = 0; i < kr.size(); i++)
    {
        if (kr[i].algorithm_state == STATE_GTO_PRESENT_DONE && (kr[i].gto_present_timer != 0 || kr[i].gto_notification_timer != 0))
            kr[i].algorithm_state = STATE_GTO_PRESENT;
        if (kr[i].algorithm_state == STATE_GTO_PRESENT)
            break;
    }

    // Mengubah state RSS_CHECK_DONE ke GTO_PRESENT
    //---------------------------------------------
    for (i = 0; i < kr.size(); i++)
    {
        if (kr[i].algorithm_state == STATE_RSS_CHECK_DONE)
            kr[i].algorithm_state = STATE_GTO_PRESENT;
        if (kr[i].algorithm_state == STATE_GTO_PRESENT)
            break;
    }

    if (i != kr.size())
    {
        // Jika kendaraan memiliki RFID valid
        //-----------------------------------
        if (kr[i].algorithm_state == STATE_GTO_PRESENT &&
            kr[i].rfid_flag == 1)
        {
            if (kr[i].gto_present_timer == 0)
            {
                // Inisialisasi timer
                kr[i].gto_present_timer = ros::Time::now().toSec();
                // Inisialisasi retry
                kr[i].gto_present_retry = 0;
            }

            //-------------------------

            // Ulangi GTO_PRESENT setiap 3 detik
            if (ros::Time::now().toSec() - kr[i].gto_present_timer >= kr[i].gto_present_retry * 3)
            {
                help.log_error("====> GTO_PRESENT %d", ++kr[i].gto_present_retry);
                help_gto_present(i);
            }

            //-------------------------

            // Mencatat rfid dan waktu saat CU melakukan present ke GTO
            if (kr[i].gto_present_retry <= 1)
            {
                rfid rfid_present_pool_buffer;
                rfid_present_pool_buffer.tid = kr[i].rfid_tid;
                rfid_present_pool_buffer.time = ros::Time::now().toSec();
                rfid_present_pool.push_back(rfid_present_pool_buffer);
            }

            //-------------------------

            // Mengubah state GTO_PRESENT ke STATE_GTO_PRESENT_DONE
            kr[i].algorithm_state = STATE_GTO_PRESENT_DONE;

            //-------------------------

            // Jika timeout tercapai
            if (ros::Time::now().toSec() - kr[i].gto_present_timer >= 3600)
            {
                help.log_error("====> Timeout 3600 detik tercapai");

                kr.pop_front();

                help.log_warn("====> Kendaraan keluar dari dalam buffer antrian");
                help.log_warn("====> %d kendaraan di dalam buffer antrian", kr.size());
            }
        }
        // Jika kendaraan memiliki RFID tidak valid
        //-----------------------------------------
        else if (kr[i].algorithm_state == STATE_GTO_PRESENT &&
                 kr[i].rfid_flag != 1)
        {
            if (kr[i].gto_notification_timer == 0)
            {
                // Inisialisasi timer
                kr[i].gto_notification_timer = ros::Time::now().toSec();
                // Inisialisasi retry
                kr[i].gto_notification_retry = 0;
                kr[i].rss_check_retry = 1;
            }

            //-------------------------

            // Ulangi GTO_NOTIFICATION setiap 20 detik
            if (ros::Time::now().toSec() - kr[i].gto_notification_timer >= kr[i].gto_notification_retry * 20)
            {
                help.log_error("====> GTO_NOTIFICATION %d", ++kr[i].gto_notification_retry);
                help_gto_notification(i);
            }

            // Ulangi RSS_CHECK setiap 3 detik
            if (ros::Time::now().toSec() - kr[i].gto_notification_timer >= kr[i].rss_check_retry * 3)
            {
                help.log_error("====> RSS_CHECK %d", kr[i].rss_check_retry++);
                help_rss_check(i);
            }

            //-------------------------

            // Mengubah state GTO_PRESENT ke GTO_PRESENT_DONE
            kr[i].algorithm_state = STATE_GTO_PRESENT_DONE;

            //-------------------------

            // Jika timeout tercapai
            if (ros::Time::now().toSec() - kr[i].gto_notification_timer >= 120)
            {
                help.log_error("====> Timeout 120 detik tercapai");

                kr.pop_front();

                help.log_warn("====> Kendaraan keluar dari dalam buffer antrian");
                help.log_warn("====> %d kendaraan di dalam buffer antrian", kr.size());
            }
        }
    }

    mutex_kr.unlock();
    tim_51hz.start();
}

// Rutin RSS_STORE
void cllbck_tim_52hz(const ros::TimerEvent &event)
{
    if (kr.size() == 0)
        return;

    tim_52hz.stop();
    mutex_kr.lock();

    int i;

    // Mengubah state GTO_STORE_DONE ke RSS_STORE
    //-------------------------------------------
    for (i = 0; i < kr.size(); i++)
    {
        if (kr[i].algorithm_state == STATE_GTO_STORE_DONE)
            kr[i].algorithm_state = STATE_RSS_STORE;
        if (kr[i].algorithm_state == STATE_RSS_STORE)
            break;
    }

    if (i != kr.size())
    {
        // Jika kendaraan memiliki RFID
        //-----------------------------
        if (kr[i].algorithm_state == STATE_RSS_STORE)
        {
            // Jika metode pembayaran RFID atau notran
            if (kr[i].metode_pembayaran == 0 || kr[i].metode_pembayaran == 3)
            {
                // Jika berhasil
                if (help_rss_store(i))
                {
                    // Menampilkan summary waktu transaksi
                    report_time(i);

                    //-------------------------

                    // Mengubah state RSS_STORE ke RSS_STORE_DONE
                    kr[i].algorithm_state = STATE_RSS_STORE_DONE;

                    //-------------------------

                    kr.pop_front();

                    help.log_warn("====> Kendaraan keluar dari dalam buffer antrian");
                    help.log_warn("====> %d kendaraan di dalam buffer antrian", kr.size());
                }
                // Jika gagal
                else
                {
                    if (++kr[i].rss_store_retry == 5)
                    {
                        // Mengubah state RSS_STORE ke RSS_STORE_DONE
                        kr[i].algorithm_state = STATE_RSS_STORE_DONE;

                        //-------------------------

                        kr.pop_front();

                        help.log_warn("====> Kendaraan keluar dari dalam buffer antrian");
                        help.log_warn("====> %d kendaraan di dalam buffer antrian", kr.size());
                    }
                }
            }
            // Jika metode pembayaran ETOLL atau Kartu Dinas
            else
            {
                // Mengubah state RSS_STORE ke RSS_STORE_DONE
                kr[i].algorithm_state = STATE_RSS_STORE_DONE;

                //-------------------------

                kr.pop_front();

                help.log_warn("====> Kendaraan keluar dari dalam buffer antrian");
                help.log_warn("====> %d kendaraan di dalam buffer antrian", kr.size());
            }
        }
    }

    mutex_kr.unlock();
    tim_52hz.start();
}

// Rutin MISC
void cllbck_tim_53hz(const ros::TimerEvent &event)
{
    // Status GTO request
    //-------------------
    if (ros::Time::now().toSec() - gto_status_timer > gto_status_interval)
    {
        gto_status_timer = ros::Time::now().toSec();

        //-----------------------------

        if (ros::Time::now().toSec() - gto_status.datetime > gto_status_interval + 0.1)
        {
            gto_status.status = 255;

            //---------------------------------

            boost::posix_time::ptime time_ptime = boost::posix_time::microsec_clock::local_time();
            std::string time_string = boost::posix_time::to_simple_string(time_ptime);

            JSON_WRITE(slff_json["gto_status"]["status"], gto_status.status);
            JSON_WRITE(slff_json["gto_status"]["datetime"], time_string);
            json_write(slff_json_path, slff_json);

            //---------------------------------

            // Menampilkan summary
            help.log_error("====> GTO_STATUS_RESPONSE Timeout");
        }

        //-----------------------------

        slff::gto_status_request msg_gto_status_request;
        pub_gto_status_request.publish(msg_gto_status_request);

        // Menampilkan summary
        help.log_warn("====> GTO_STATUS_REQUEST Success");
    }

    //=========================================================================

    // Status RFID request
    //--------------------
    if (ros::Time::now().toSec() - rfid_status_timer > rfid_status_interval)
    {
        rfid_status_timer = ros::Time::now().toSec();

        //-----------------------------

        if (ros::Time::now().toSec() - rfid_status.datetime > rfid_status_interval + 0.1)
        {
            rfid_status.status = 255;

            //---------------------------------

            boost::posix_time::ptime time_ptime = boost::posix_time::microsec_clock::local_time();
            std::string time_string = boost::posix_time::to_simple_string(time_ptime);

            JSON_WRITE(slff_json["rfid_status"]["status"], rfid_status.status);
            JSON_WRITE(slff_json["rfid_status"]["datetime"], time_string);
            json_write(slff_json_path, slff_json);

            //---------------------------------

            // Menampilkan summary
            help.log_error("====> RFID_STATUS_RESPONSE Timeout");
        }

        //-----------------------------

        slff::rfid_status_request msg_rfid_status_request;
        pub_rfid_status_request.publish(msg_rfid_status_request);

        // Menampilkan summary
        help.log_warn("====> RFID_STATUS_REQUEST Success");
    }
}

// Rutin MISC
void cllbck_tim_1hz(const ros::TimerEvent &event)
{
    slff::exporter_peripheral_status msg_exporter_peripheral_status;
    msg_exporter_peripheral_status.gto = gto_status.status;
    msg_exporter_peripheral_status.rfid = rfid_status.status;
    pub_exporter_peripheral_status.publish(msg_exporter_peripheral_status);

    slff::exporter_version msg_exporter_version;
    msg_exporter_version.version = std::string(version);
    pub_exporter_version.publish(msg_exporter_version);
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void cllbck_sub_rfid_tag(const slff::rfid_tagConstPtr &msg)
{
    mutex_kr.lock();

    byte_array_to_hex_string(rfid_epc, (uint8_t *)msg->epc.data(), msg->epc.size());
    byte_array_to_hex_string(rfid_tid, (uint8_t *)msg->tid.data(), msg->tid.size());
    byte_array_to_hex_string(rfid_userdata, (uint8_t *)msg->userdata.data(), msg->userdata.size());

    help.log_warn("====> RFID Received");
    help.log_info("TID      = %s", rfid_tid.c_str());
    help.log_info("EPC      = %s", rfid_epc.c_str());
    help.log_info("UserData = %s", rfid_userdata.c_str());

    // Jika enkripsi TID diperiksa
    if (auth_rfid)
    {
        std::string signature = rfid_epc.substr(0, 4);

        if (signature == "1321")
        {
            help.log_info("Status   = Maintenance");
            help_cdp("RFID\nMNTNNC");
        }
        else if (signature == "1378")
        {
            // Key yang masuk
            std::string keyA = rfid_epc.substr(4, 20);

            // Key yang seharusnya
            std::string keyB = rfid_encrypt(rfid_tid.substr(4, 20));

            // Memeriksa key
            if (keyA != keyB)
            {
                help.log_info("Status   = Transaksi tidak valid");
                help_cdp("RFID\nINVALID");
            }
            else
            {
                help.log_info("Status   = Transaksi valid");
                queue_entry();
            }
        }
        else
        {
            help.log_info("Status   = Bukan milik Jasamarga");
        }
    }
    // Jika enkripsi TID tidak diperiksa
    else
    {
        std::string signature = rfid_epc.substr(0, 4);

        if (signature == "1321")
        {
            help.log_info("Status   = Maintenance");
            help_cdp("RFID\nMNTNNC");
        }
        else
        {
            help.log_info("Status   = Transaksi valid");
            queue_entry();
        }
    }

    mutex_kr.unlock();
}

void cllbck_sub_gto_store(const slff::gto_storeConstPtr &msg)
{
    if (kr.size() == 0)
    {
        // Menampilkan summary
        help.log_warn("====> GTO_STORE Success");
        report_gto_store(*msg);
        help_gto_ack_ok();

        return;
    }

    mutex_kr.lock();

    int i;

    // Mengubah state GTO_PRESENT_DONE ke GTO_STORE
    //---------------------------------------------
    for (i = 0; i < kr.size(); i++)
    {
        if (kr[i].algorithm_state == STATE_GTO_PRESENT || kr[i].algorithm_state == STATE_GTO_PRESENT_DONE)
            kr[i].algorithm_state = STATE_GTO_STORE;
        if (kr[i].algorithm_state == STATE_GTO_STORE)
            break;
    }

    if (i != kr.size())
    {
        // Jika kendaraan memiliki RFID
        //-----------------------------
        if (kr[i].algorithm_state == STATE_GTO_STORE)
        {
            // Jika RFID tidak valid
            if (kr[i].rfid_flag != 1)
            {
                help_gto_store(i, *msg);
            }
            // Jika RFID valid
            else if (kr[i].rfid_flag == 1)
            {
                if (msg->no_seri_control_unit == kr[i].no_seri_control_unit &&
                    msg->rfid_tid == kr[i].rfid_tid)
                    help_gto_store(i, *msg);
            }

            //-----------------------------

            // Mengubah state GTO_STORE ke RSS_STORE
            kr[i].algorithm_state = STATE_RSS_STORE;
        }
    }

    mutex_kr.unlock();
}

void cllbck_sub_gto_init(const slff::gto_initConstPtr &msg)
{
    mutex_kr.lock();

    help_gto_init(*msg);

    mutex_kr.unlock();
}

void cllbck_sub_gto_status_response(const slff::gto_status_responseConstPtr &msg)
{
    gto_status.status = msg->status == 0 ? 0 : 1;
    gto_status.datetime = ros::Time::now().toSec();

    //---------------------------------

    boost::posix_time::ptime time_ptime = boost::posix_time::microsec_clock::local_time();
    std::string time_string = boost::posix_time::to_simple_string(time_ptime);

    JSON_WRITE(slff_json["gto_status"]["status"], gto_status.status);
    JSON_WRITE(slff_json["gto_status"]["datetime"], time_string);
    json_write(slff_json_path, slff_json);

    //---------------------------------

    // Menampilkan summary
    help.log_warn("====> GTO_STATUS_RESPONSE Success");
    help.log_info("status = %d", gto_status.status);
    help_gto_ack_ok();
}

void cllbck_sub_rfid_status_response(const slff::rfid_status_responseConstPtr &msg)
{
    rfid_status.status = msg->status == 0 ? 0 : 1;
    rfid_status.datetime = ros::Time::now().toSec();

    //---------------------------------

    boost::posix_time::ptime time_ptime = boost::posix_time::microsec_clock::local_time();
    std::string time_string = boost::posix_time::to_simple_string(time_ptime);

    JSON_WRITE(slff_json["rfid_status"]["status"], rfid_status.status);
    JSON_WRITE(slff_json["rfid_status"]["datetime"], time_string);
    json_write(slff_json_path, slff_json);

    //---------------------------------

    // Menampilkan summary
    help.log_warn("====> RFID_STATUS_RESPONSE Success");
    help.log_info("status = %d", rfid_status.status);
    help_rfid_ack_ok();
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

int routine_init()
{
    ros::Duration(2.5).sleep();

    help.log_warn(R"(
 _______________________
/ Single Lane Free Flow \
\ IoT Lab Jasa Marga    /
 -----------------------
        \   ^__^
         \  (oo)\_______
            (__)\       )\/\
                ||----w |
                ||     ||
)");

    slff_json_path = getenv("HOME") + std::string("/slff-data/slff.json");

    if (!boost::filesystem::exists(slff_json_path))
    {
        // Version -------------------------------
        JSON_WRITE(slff_json["version"], version);
        json_write(slff_json_path, slff_json);
        //----------------------------------------

        JSON_WRITE(slff_json["no_seri_control_unit"], no_seri_control_unit);
        json_write(slff_json_path, slff_json);
    }
    else
    {
        slff_json = json_read(slff_json_path);
        JSON_READ(slff_json["no_seri_control_unit"], no_seri_control_unit);

        // Version -------------------------------
        JSON_WRITE(slff_json["version"], version);
        json_write(slff_json_path, slff_json);
        //----------------------------------------
    }

    gto_status.status = rfid_status.status = 255;
    gto_status.datetime = rfid_status.datetime = ros::Time::now().toSec();
    gto_status_timer = rfid_status_timer = ros::Time::now().toSec();

    return 0;
}

int routine_routine()
{
    return 0;
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

bool help_rss_check(uint8_t n)
{
    slff::rss_check srv_rss_check;
    // RFID TID
    srv_rss_check.request.rfid_tid = kr[n].rfid_tid;
    cli_rss_check.call(srv_rss_check);

    kr[n].time_rss_check = ros::Time::now().toSec();

    if (srv_rss_check.response.success)
    {
        nlohmann::json j = nlohmann::json::parse(srv_rss_check.response.response);

        // RFID flag
        JSON_READ(j["status_code"], kr[n].rfid_flag);
        // Golongan, jenis, dan plat no kendaraan
        JSON_READ_ATOI(j["data"]["golongan"], kr[n].golongan_kendaraan);
        JSON_READ_ATOI(j["data"]["jenis_kendaraan"], kr[n].jenis_kendaraan);
        JSON_READ(j["data"]["no_kendaraan"], kr[n].plat_no_rss);
        // Saldo dan tarif
        JSON_READ_ATOI(j["data"]["saldo"], kr[n].saldo);
        JSON_READ_ATOI(j["data"]["tarif"], kr[n].tarif);
        // No gardu dan no gerbang entrance
        JSON_READ_ATOI(j["data"]["entrance_gardu_id"], kr[n].no_gardu_entrance);
        JSON_READ_ATOI(j["data"]["entrance_gerbang_id"], kr[n].no_gerbang_entrance);
        // DateTime entrance
        if (!j["data"]["entrance_time"].is_null())
        {
            std::string entrance_time_buffer = j["data"]["entrance_time"];
            if (entrance_time_buffer.length() == 19)
            {
                kr[n].entrance_year = std::stoi(std::string(j["data"]["entrance_time"]).substr(2, 2));
                kr[n].entrance_month = std::stoi(std::string(j["data"]["entrance_time"]).substr(5, 2));
                kr[n].entrance_day = std::stoi(std::string(j["data"]["entrance_time"]).substr(8, 2));
                kr[n].entrance_hour = std::stoi(std::string(j["data"]["entrance_time"]).substr(11, 2));
                kr[n].entrance_minute = std::stoi(std::string(j["data"]["entrance_time"]).substr(14, 2));
                kr[n].entrance_second = std::stoi(std::string(j["data"]["entrance_time"]).substr(17, 2));
            }
        }
        // DateTime transaksi
        if (!j["data"]["trans_date"].is_null())
        {
            std::string trans_date_buffer = j["data"]["trans_date"];
            if (trans_date_buffer.length() == 19)
            {
                kr[n].transaction_year = std::stoi(std::string(j["data"]["trans_date"]).substr(2, 2));
                kr[n].transaction_month = std::stoi(std::string(j["data"]["trans_date"]).substr(5, 2));
                kr[n].transaction_day = std::stoi(std::string(j["data"]["trans_date"]).substr(8, 2));
                kr[n].transaction_hour = std::stoi(std::string(j["data"]["trans_date"]).substr(11, 2));
                kr[n].transaction_minute = std::stoi(std::string(j["data"]["trans_date"]).substr(14, 2));
                kr[n].transaction_second = std::stoi(std::string(j["data"]["trans_date"]).substr(17, 2));
            }
        }
        // Hash
        JSON_READ(j["data"]["hash"], kr[n].hash_rss);

        //-----------------------------

        // Menampilkan summary
        help.log_warn("====> RSS_CHECK Success for %d | %s", kr[n].no_seri_control_unit, kr[n].rfid_tid.c_str());
        help.log_warn("====> RSS_CHECK Response for %d | %s\n%s", kr[n].no_seri_control_unit, kr[n].rfid_tid.c_str(), srv_rss_check.response.response.c_str());
        report_rss_check(n, srv_rss_check);

        return true;
    }
    else
    {
        // Menampilkan summary
        help.log_error("====> RSS_CHECK Failed for %d | %s", kr[n].no_seri_control_unit, kr[n].rfid_tid.c_str());
        help.log_error("====> RSS_CHECK Response for %d | %s\n%s", kr[n].no_seri_control_unit, kr[n].rfid_tid.c_str(), srv_rss_check.response.response.c_str());
        // report_rss_check(n, srv_rss_check);

        return false;
    }
}

bool help_rss_store(uint8_t n)
{
    slff::rss_store srv_rss_store;
    // Metode pembayaran
    srv_rss_store.request.metode_pembayaran = kr[n].metode_pembayaran;
    // RFID flag dan TID
    srv_rss_store.request.rfid_flag = kr[n].rfid_flag;
    srv_rss_store.request.rfid_tid = kr[n].rfid_tid;
    // No gardu, no gerbang, DateTime entrance dan DateTime exit
    srv_rss_store.request.no_gardu_entrance = kr[n].no_gardu_entrance;
    srv_rss_store.request.no_gerbang_entrance = kr[n].no_gerbang_entrance;
    srv_rss_store.request.no_gardu_exit = kr[n].no_gardu_exit;
    srv_rss_store.request.no_gerbang_exit = kr[n].no_gerbang_exit;
    srv_rss_store.request.entrance_year = kr[n].entrance_year;
    srv_rss_store.request.entrance_month = kr[n].entrance_month;
    srv_rss_store.request.entrance_day = kr[n].entrance_day;
    srv_rss_store.request.entrance_hour = kr[n].entrance_hour;
    srv_rss_store.request.entrance_minute = kr[n].entrance_minute;
    srv_rss_store.request.entrance_second = kr[n].entrance_second;
    srv_rss_store.request.exit_year = kr[n].exit_year;
    srv_rss_store.request.exit_month = kr[n].exit_month;
    srv_rss_store.request.exit_day = kr[n].exit_day;
    srv_rss_store.request.exit_hour = kr[n].exit_hour;
    srv_rss_store.request.exit_minute = kr[n].exit_minute;
    srv_rss_store.request.exit_second = kr[n].exit_second;
    // DateTime transaksi
    srv_rss_store.request.transaction_year = kr[n].transaction_year;
    srv_rss_store.request.transaction_month = kr[n].transaction_month;
    srv_rss_store.request.transaction_day = kr[n].transaction_day;
    srv_rss_store.request.transaction_hour = kr[n].transaction_hour;
    srv_rss_store.request.transaction_minute = kr[n].transaction_minute;
    srv_rss_store.request.transaction_second = kr[n].transaction_second;
    // Date laporan
    srv_rss_store.request.report_year = kr[n].report_year;
    srv_rss_store.request.report_month = kr[n].report_month;
    srv_rss_store.request.report_day = kr[n].report_day;
    // Kode ruas
    srv_rss_store.request.kode_ruas = kr[n].kode_ruas;
    // No shift, perioda, resi, KSPT, dan PLT
    srv_rss_store.request.no_shift = kr[n].no_shift;
    srv_rss_store.request.no_perioda = kr[n].no_perioda;
    srv_rss_store.request.no_resi = kr[n].no_resi;
    srv_rss_store.request.no_kspt = kr[n].no_kspt;
    srv_rss_store.request.no_plt = kr[n].no_plt;
    // Hash
    srv_rss_store.request.hash = kr[n].hash_rss;
    cli_rss_store.call(srv_rss_store);

    kr[n].time_rss_store = ros::Time::now().toSec();

    if (srv_rss_store.response.success)
    {
        nlohmann::json j = nlohmann::json::parse(srv_rss_store.response.response);

        //-----------------------------

        // Menampilkan summary
        help.log_warn("====> RSS_STORE Success for %d | %s", kr[n].no_seri_control_unit, kr[n].rfid_tid.c_str());
        help.log_warn("====> RSS_STORE Response for %d | %s\n%s", kr[n].no_seri_control_unit, kr[n].rfid_tid.c_str(), srv_rss_store.response.response.c_str());
        report_rss_store(n, srv_rss_store);

        return true;
    }
    else
    {
        // Menampilkan summary
        help.log_error("====> RSS_STORE Failed for %d | %s", kr[n].no_seri_control_unit, kr[n].rfid_tid.c_str());
        help.log_error("====> RSS_STORE Response for %d | %s\n%s", kr[n].no_seri_control_unit, kr[n].rfid_tid.c_str(), srv_rss_store.response.response.c_str());
        // report_rss_store(n, srv_rss_store);

        return false;
    }
}

void help_gto_present(uint8_t n)
{
    slff::gto_present msg_gto_present;
    // No seri control unit
    msg_gto_present.no_seri_control_unit = kr[n].no_seri_control_unit;
    // RFID TID
    msg_gto_present.rfid_tid = kr[n].rfid_tid;
    // Golongan, jenis, dan plat no kendaraan
    msg_gto_present.golongan_kendaraan = kr[n].golongan_kendaraan;
    msg_gto_present.jenis_kendaraan = kr[n].jenis_kendaraan;
    msg_gto_present.plat_no_rss = kr[n].plat_no_rss;
    msg_gto_present.plat_no_anpr = kr[n].plat_no_anpr;
    // Saldo dan tarif
    msg_gto_present.saldo = kr[n].saldo - kr[n].tarif < 0 ? 0 : kr[n].saldo - kr[n].tarif;
    msg_gto_present.tarif = kr[n].tarif;
    // No gardu dan no gerbang entrance
    msg_gto_present.no_gardu_entrance = kr[n].no_gardu_entrance;
    msg_gto_present.no_gerbang_entrance = kr[n].no_gerbang_entrance;
    // No gardu dan no gerbang exit
    msg_gto_present.no_gardu_exit = kr[n].no_gardu_exit;
    msg_gto_present.no_gerbang_exit = kr[n].no_gerbang_exit;
    // DateTime entrance
    msg_gto_present.entrance_year = kr[n].entrance_year;
    msg_gto_present.entrance_month = kr[n].entrance_month;
    msg_gto_present.entrance_day = kr[n].entrance_day;
    msg_gto_present.entrance_hour = kr[n].entrance_hour;
    msg_gto_present.entrance_minute = kr[n].entrance_minute;
    msg_gto_present.entrance_second = kr[n].entrance_second;
    // Hash
    msg_gto_present.hash = kr[n].hash_rss;
    pub_gto_present.publish(msg_gto_present);

    kr[n].time_gto_present = ros::Time::now().toSec();

    //---------------------------------

    // Menampilkan summary
    help.log_warn("====> GTO_PRESENT Success for %d | %s", kr[n].no_seri_control_unit, kr[n].rfid_tid.c_str());
    report_gto_present(msg_gto_present);
}

void help_gto_notification(uint8_t n)
{
    slff::gto_notification msg_gto_notification;
    // No seri control unit
    msg_gto_notification.no_seri_control_unit = kr[n].no_seri_control_unit;
    // RFID TID
    msg_gto_notification.rfid_tid = kr[n].rfid_tid;
    // Golongan kendaraan
    msg_gto_notification.golongan_kendaraan = kr[n].golongan_kendaraan;
    // Message
    switch (kr[n].rfid_flag)
    {
    case 2: msg_gto_notification.message = "RFID\nUNREG"; break;
    case 3: msg_gto_notification.message = "BLACKLIST\nUSER"; break;
    case 4: msg_gto_notification.message = "BLACKLIST\nFLO"; break;
    case 5: msg_gto_notification.message = "SALDO\nKURANG"; break;
    case 6: msg_gto_notification.message = "ENTRANCE\nNOT\nFOUND"; break;
    case 7: msg_gto_notification.message = "ENTRANCE\nTIME\nEXPIRED"; break;
    case 8: msg_gto_notification.message = "TARIFF\nNOT\nFOUND"; break;
    default: msg_gto_notification.message = "FLO\nOUT OF\nSERVICE"; break;
    }
    pub_gto_notification.publish(msg_gto_notification);

    kr[n].time_gto_present = ros::Time::now().toSec();

    //---------------------------------

    // Menampilkan summary
    help.log_warn("====> GTO_NOTIFICATION Success for %d | %s", kr[n].no_seri_control_unit, kr[n].rfid_tid.c_str());
    report_gto_notification(msg_gto_notification);
}

void help_gto_store(uint8_t n, slff::gto_store msg)
{
    kr[n].metode_pembayaran = msg.metode_pembayaran;
    kr[n].entrance_year = msg.entrance_year;
    kr[n].entrance_month = msg.entrance_month;
    kr[n].entrance_day = msg.entrance_day;
    kr[n].entrance_hour = msg.entrance_hour;
    kr[n].entrance_minute = msg.entrance_minute;
    kr[n].entrance_second = msg.entrance_second;
    kr[n].exit_year = msg.exit_year;
    kr[n].exit_month = msg.exit_month;
    kr[n].exit_day = msg.exit_day;
    kr[n].exit_hour = msg.exit_hour;
    kr[n].exit_minute = msg.exit_minute;
    kr[n].exit_second = msg.exit_second;
    kr[n].report_year = msg.report_year;
    kr[n].report_month = msg.report_month;
    kr[n].report_day = msg.report_day;
    kr[n].kode_ruas = msg.kode_ruas;
    kr[n].no_shift = msg.no_shift;
    kr[n].no_perioda = msg.no_perioda;
    kr[n].no_resi = msg.no_resi;
    kr[n].no_kspt = msg.no_kspt;
    kr[n].no_plt = msg.no_plt;
    kr[n].hash_gto = msg.hash;

    kr[n].time_gto_store = ros::Time::now().toSec();

    //---------------------------------

    // Menampilkan summary
    help.log_warn("====> GTO_STORE Success for %d | %s", msg.no_seri_control_unit, msg.rfid_tid.c_str());
    report_gto_store(msg);
    help_gto_ack_ok();
}

void help_gto_init(slff::gto_init msg)
{
    kr.clear();

    //---------------------------------

    // Menampilkan summary
    help.log_warn("====> GTO_INIT Success");
    report_gto_init(msg);
    help_gto_ack_ok();
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void report_rss_check(uint8_t n, slff::rss_check srv)
{
    help.log_info("====> INPUT");
    help.log_info("rfid_tid             = %s", srv.request.rfid_tid.c_str());
    help.log_info("====> OUTPUT");
    help.log_info("rfid_flag            = %d", kr[n].rfid_flag);
    help.log_info("golongan_kendaraan   = %d", kr[n].golongan_kendaraan);
    help.log_info("jenis_kendaraan      = %d", kr[n].jenis_kendaraan);
    help.log_info("plat_no_rss          = %s", kr[n].plat_no_rss.c_str());
    help.log_info("saldo                = %d", kr[n].saldo);
    help.log_info("tarif                = %d", kr[n].tarif);
    help.log_info("no_gardu_entrance    = %d (hanya valid pada gardu entrance)", kr[n].no_gardu_entrance);
    help.log_info("no_gerbang_entrance  = %d (hanya valid pada gardu entrance)", kr[n].no_gerbang_entrance);
    help.log_info("entrance_datetime    = %04d-%02d-%02d %02d:%02d:%02d (hanya valid pada gardu entrance)", kr[n].entrance_year + 2000, kr[n].entrance_month, kr[n].entrance_day, kr[n].entrance_hour, kr[n].entrance_minute, kr[n].entrance_second);
    help.log_info("transaction_datetime = %04d-%02d-%02d %02d:%02d:%02d", kr[n].transaction_year + 2000, kr[n].transaction_month, kr[n].transaction_day, kr[n].transaction_hour, kr[n].transaction_minute, kr[n].transaction_second);
    help.log_info("hash                 = %s", kr[n].hash_rss.c_str());
}

void report_rss_store(uint8_t n, slff::rss_store srv)
{
    help.log_info("====> INPUT");
    help.log_info("metode_pembayaran    = %d", srv.request.metode_pembayaran);
    help.log_info("rfid_flag            = %d", srv.request.rfid_flag);
    help.log_info("rfid_tid             = %s", srv.request.rfid_tid.c_str());
    help.log_info("no_gardu_entrance    = %d (hanya valid pada gardu entrance)", srv.request.no_gardu_entrance);
    help.log_info("no_gerbang_entrance  = %d (hanya valid pada gardu entrance)", srv.request.no_gerbang_entrance);
    help.log_info("no_gardu_exit        = %d", srv.request.no_gardu_exit);
    help.log_info("no_gerbang_exit      = %d", srv.request.no_gerbang_exit);
    help.log_info("entrance_datetime    = %04d-%02d-%02d %02d:%02d:%02d (hanya valid pada gardu entrance)", srv.request.entrance_year + 2000, srv.request.entrance_month, srv.request.entrance_day, srv.request.entrance_hour, srv.request.entrance_minute, srv.request.entrance_second);
    help.log_info("exit_datetime        = %04d-%02d-%02d %02d:%02d:%02d", srv.request.exit_year + 2000, srv.request.exit_month, srv.request.exit_day, srv.request.exit_hour, srv.request.exit_minute, srv.request.exit_second);
    help.log_info("transaction_datetime = %04d-%02d-%02d %02d:%02d:%02d", srv.request.transaction_year + 2000, srv.request.transaction_month, srv.request.transaction_day, srv.request.transaction_hour, srv.request.transaction_minute, srv.request.transaction_second);
    help.log_info("report_date          = %04d-%02d-%02d", srv.request.report_year + 2000, srv.request.report_month, srv.request.report_day);
    help.log_info("kode_ruas            = %d", srv.request.kode_ruas);
    help.log_info("no_shift             = %d", srv.request.no_shift);
    help.log_info("no_perioda           = %d", srv.request.no_perioda);
    help.log_info("no_resi              = %d", srv.request.no_resi);
    help.log_info("no_kspt              = %d", srv.request.no_kspt);
    help.log_info("no_plt               = %d", srv.request.no_plt);
    help.log_info("hash                 = %s", srv.request.hash.c_str());
}

void report_gto_present(slff::gto_present msg)
{
    help.log_info("no_seri_control_unit = %d", msg.no_seri_control_unit);
    help.log_info("rfid_tid             = %s", msg.rfid_tid.c_str());
    help.log_info("golongan_kendaraan   = %d", msg.golongan_kendaraan);
    help.log_info("jenis_kendaraan      = %d", msg.jenis_kendaraan);
    help.log_info("plat_no_rss          = %s", msg.plat_no_rss.c_str());
    help.log_info("plat_no_anpr         = %s", msg.plat_no_anpr.c_str());
    help.log_info("saldo                = %d", msg.saldo);
    help.log_info("tarif                = %d", msg.tarif);
    help.log_info("no_gardu_entrance    = %d (hanya valid pada gardu entrance)", msg.no_gardu_entrance);
    help.log_info("no_gerbang_entrance  = %d (hanya valid pada gardu entrance)", msg.no_gerbang_entrance);
    help.log_info("no_gardu_exit        = %d", msg.no_gardu_exit);
    help.log_info("no_gerbang_exit      = %d", msg.no_gerbang_exit);
    help.log_info("entrance_datetime    = %04d-%02d-%02d %02d:%02d:%02d (hanya valid pada gardu entrance)", msg.entrance_year + 2000, msg.entrance_month, msg.entrance_day, msg.entrance_hour, msg.entrance_minute, msg.entrance_second);
    help.log_info("hash                 = %s", msg.hash.c_str());
}

void report_gto_notification(slff::gto_notification msg)
{
    help.log_info("no_seri_control_unit = %d", msg.no_seri_control_unit);
    help.log_info("rfid_tid             = %s", msg.rfid_tid.c_str());
    help.log_info("golongan_kendaraan   = %d", msg.golongan_kendaraan);
    std::replace(msg.message.begin(), msg.message.end(), '\n', ' ');
    help.log_info("message              = %s", msg.message.c_str());
}

void report_gto_store(slff::gto_store msg)
{
    help.log_info("no_seri_control_unit = %d", msg.no_seri_control_unit);
    help.log_info("metode_pembayaran    = %d", msg.metode_pembayaran);
    help.log_info("rfid_tid             = %s", msg.rfid_tid.c_str());
    help.log_info("entrance_datetime    = %04d-%02d-%02d %02d:%02d:%02d (hanya valid pada gardu entrance)", msg.entrance_year + 2000, msg.entrance_month, msg.entrance_day, msg.entrance_hour, msg.entrance_minute, msg.entrance_second);
    help.log_info("exit_datetime        = %04d-%02d-%02d %02d:%02d:%02d", msg.exit_year + 2000, msg.exit_month, msg.exit_day, msg.exit_hour, msg.exit_minute, msg.exit_second);
    help.log_info("report_date          = %04d-%02d-%02d", msg.report_year + 2000, msg.report_month, msg.report_day);
    help.log_info("kode_ruas            = %d", msg.kode_ruas);
    help.log_info("no_shift             = %d", msg.no_shift);
    help.log_info("no_perioda           = %d", msg.no_perioda);
    help.log_info("no_resi              = %d", msg.no_resi);
    help.log_info("no_kspt              = %d", msg.no_kspt);
    help.log_info("no_plt               = %d", msg.no_plt);
    help.log_info("hash                 = %s", msg.hash.c_str());
}

void report_gto_init(slff::gto_init msg)
{
    help.log_info("no_shift   = %d", msg.no_shift);
    help.log_info("no_perioda = %d", msg.no_perioda);
    help.log_info("no_resi    = %d", msg.no_resi);
    help.log_info("no_kspt    = %d", msg.no_kspt);
    help.log_info("no_plt     = %d", msg.no_plt);
}

void report_time(uint8_t n)
{
    help.log_warn("====> Rangkuman waktu transaksi");
    help.log_info("Detect -> RSS Check   = %03.0lfms                ", (kr[n].time_rss_check - kr[n].time_detect) * 1000);
    help.log_info("Detect -> GTO Present = %03.0lfms, dt = %03.0lfms", (kr[n].time_gto_present - kr[n].time_detect) * 1000, (kr[n].time_gto_present - kr[n].time_rss_check) * 1000);
    help.log_info("Detect -> GTO Store   = %03.0lfms, dt = %03.0lfms", (kr[n].time_gto_store - kr[n].time_detect) * 1000, (kr[n].time_gto_store - kr[n].time_gto_present) * 1000);
    help.log_info("Detect -> RSS Store   = %03.0lfms, dt = %03.0lfms", (kr[n].time_rss_store - kr[n].time_detect) * 1000, (kr[n].time_rss_store - kr[n].time_gto_store) * 1000);
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void help_gto_ack_ok()
{
    slff::gto_ack msg_gto_ack;
    msg_gto_ack.status = 0;
    pub_gto_ack.publish(msg_gto_ack);

    // Menampilkan summary
    help.log_warn("====> GTO_ACK Success");
    help.log_info("status = %d", msg_gto_ack.status);
}

void help_gto_ack_error()
{
    slff::gto_ack msg_gto_ack;
    msg_gto_ack.status = 1;
    pub_gto_ack.publish(msg_gto_ack);

    // Menampilkan summary
    help.log_warn("====> GTO_ACK Success");
    help.log_info("status = %d", msg_gto_ack.status);
}

void help_rfid_ack_ok()
{
    slff::rfid_ack msg_rfid_ack;
    msg_rfid_ack.status = 0;
    pub_rfid_ack.publish(msg_rfid_ack);

    // Menampilkan summary
    help.log_warn("====> RFID_ACK Success");
    help.log_info("status = %d", msg_rfid_ack.status);
}

void help_rfid_ack_error()
{
    slff::rfid_ack msg_rfid_ack;
    msg_rfid_ack.status = 1;
    pub_rfid_ack.publish(msg_rfid_ack);

    // Menampilkan summary
    help.log_warn("====> RFID_ACK Success");
    help.log_info("status = %d", msg_rfid_ack.status);
}

void help_cdp(const char *message)
{
    slff::gto_notification msg_gto_notification;
    msg_gto_notification.message = std::string(message);
    pub_gto_notification.publish(msg_gto_notification);

    // Menampilkan summary
    help.log_warn("====> GTO_NOTIFICATION Success");
    std::replace(msg_gto_notification.message.begin(), msg_gto_notification.message.end(), '\n', ' ');
    help.log_info("message = %s", msg_gto_notification.message.c_str());
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void queue_entry()
{
    uint8_t kr_new = 0;

    // Filter jumlah kendaraan ayng dapat berada di dalam antrian
    //===========================================================

    // Memeriksa apakah ada RFID yang sama di dalam antrian
    for (int i = 0; i < kr.size(); i++)
        if (rfid_tid == kr[i].rfid_tid)
        {
            help.log_error("Kendaraan dengan RFID yang sama sudah di dalam antrian");
            return;
        }

    // Memeriksa apakah jumlah antrian lebih dari atau sama dengan 2
    for (int i = 0; i < kr.size(); i++)
        if (kr[i].algorithm_state == STATE_IDDLE ||
            kr[i].algorithm_state == STATE_RSS_CHECK ||
            kr[i].algorithm_state == STATE_RSS_CHECK_DONE ||
            kr[i].algorithm_state == STATE_GTO_PRESENT ||
            kr[i].algorithm_state == STATE_GTO_PRESENT_DONE ||
            kr[i].algorithm_state == STATE_GTO_STORE ||
            kr[i].algorithm_state == STATE_GTO_STORE_DONE)
        {
            if (++kr_new >= 2)
            {
                help.log_error("Antrian kendaraan sudah penuh");
                return;
            }
        }

    // Filter kendaraan yang baru saja melakukan transaksi
    //====================================================

    // Menghapus rfid yang umurnya lebih dari 120 detik
    for (int i = 0; i < rfid_present_pool.size(); i++)
        if (ros::Time::now().toSec() - rfid_present_pool[i].time > 120)
        {
            rfid_present_pool.erase(rfid_present_pool.begin() + i--);
            continue;
        }

    // Memeriksa apakah kendaraan sudah melakukan transaksi kurang dari 120 detik yang lalu
    for (int i = 0; i < rfid_present_pool.size(); i++)
        if (rfid_tid == rfid_present_pool[i].tid)
        {
            help.log_error("Kendaraan dengan RFID yang sama sudah melakukan transaksi");
            return;
        }

    vehicle kr_buffer;
    kr_buffer.algorithm_state = STATE_IDDLE;
    kr_buffer.time_detect = ros::Time::now().toSec();
    kr_buffer.no_seri_control_unit = no_seri_control_unit;
    kr_buffer.rfid_status = 1;
    kr_buffer.rfid_flag = 0;
    kr_buffer.rfid_epc = rfid_epc;
    kr_buffer.rfid_tid = rfid_tid;
    kr_buffer.rfid_userdata = rfid_userdata;
    kr_buffer.no_gardu_entrance = kr_buffer.no_gardu_exit = no_gardu;
    kr_buffer.no_gerbang_entrance = kr_buffer.no_gerbang_exit = no_gerbang;
    kr.push_back(kr_buffer);

    JSON_WRITE(slff_json["no_seri_control_unit"], ++no_seri_control_unit);
    json_write(slff_json_path, slff_json);

    help.log_warn("====> Kendaraan masuk ke dalam buffer antrian");
    help.log_warn("====> %d kendaraan di dalam buffer antrian", kr.size());
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

std::string rfid_encrypt(std::string input)
{
    std::string str = boost::algorithm::unhex(input);

    uint8_t *data8 = (uint8_t *)str.data();
    uint16_t *data16 = (uint16_t *)str.data();
    uint8_t data8_size = str.size() / sizeof(uint8_t);
    uint8_t data16_size = str.size() / sizeof(uint16_t);

    for (int i = 0; i < data16_size; i++)
    {
        // Menukar LSB dan MSB
        data16[i] = (data16[i] << 8) | (data16[i] >> 8);
        // XOR data dengan 0x1378
        data16[i] ^= 0x1378;
        // Menukar LSB dan MSB
        data16[i] = (data16[i] << 8) | (data16[i] >> 8);
    }

    // Circular shift left 50 bit
    for (int i = 50; i > 0; i -= 8)
    {
        if (i > 8)
        {
            uint8_t tmp = data8[0];
            for (int j = 0; j < data8_size - 1; j++)
                data8[j] = data8[j + 1];
            data8[data8_size - 1] = tmp;
        }
        else
        {
            uint8_t tmp = data8[0];
            for (int j = 0; j < data8_size - 1; j++)
                data8[j] = (data8[j] << i) | (data8[j + 1] >> (8 - i));
            data8[data8_size - 1] = (data8[data8_size - 1] << i) | (tmp >> (8 - i));
        }
    }

    return boost::algorithm::hex(str);
}

std::string rfid_decrypt(std::string input)
{
    std::string str = boost::algorithm::unhex(input);

    uint8_t *data8 = (uint8_t *)str.data();
    uint16_t *data16 = (uint16_t *)str.data();
    uint8_t data8_size = str.size() / sizeof(uint8_t);
    uint8_t data16_size = str.size() / sizeof(uint16_t);

    // Circular shift right 50 bit
    for (int i = 0; i < 50; i++)
    {
        if (i > 8)
        {
            uint8_t tmp = data8[data8_size - 1];
            for (int j = data8_size - 1; j > 0; j--)
                data8[j] = data8[j - 1];
            data8[0] = tmp;
        }
        else
        {
            uint8_t tmp = data8[data8_size - 1];
            for (int j = data8_size - 1; j > 0; j--)
                data8[j] = (data8[j] >> i) | (data8[j - 1] << (8 - i));
            data8[0] = (data8[0] >> i) | (tmp << (8 - i));
        }
    }

    for (int i = 0; i < data16_size; i++)
    {
        // Menukar LSB dan MSB
        data16[i] = (data16[i] << 8) | (data16[i] >> 8);
        // XOR data dengan 0x1378
        data16[i] ^= 0x1378;
        // Menukar LSB dan MSB
        data16[i] = (data16[i] << 8) | (data16[i] >> 8);
    }

    return boost::algorithm::hex(str);
}
