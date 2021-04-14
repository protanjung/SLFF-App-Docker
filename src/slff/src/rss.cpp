#include "curl/curl.h"
#include "ros/ros.h"
#include "slff/define.h"
#include "slff/misc.h"
#include "slff/rss_check.h"
#include "slff/rss_store.h"
#include "json/json.hpp"

//=====Prototype
bool cllbck_ser_rss_check(slff::rss_check::Request &req, slff::rss_check::Response &res);
bool cllbck_ser_rss_store(slff::rss_store::Request &req, slff::rss_store::Response &res);

int rss_init();

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

//=====Parameter
std::string rss_check_url;
std::string rss_store_url;
std::string rss_jm_code;
int tipe_control_unit;
std::string tid;
std::string mid;
//=====ServiceServer
ros::ServiceServer ser_rss_check;
ros::ServiceServer ser_rss_store;
//=====Help
misc help;

//=====CURL
struct memory
{
    char *response;
    size_t size;
};

struct memory chunk;

int main(int argc, char **argv)
{
    ros::init(argc, argv, "rss");

    ros::NodeHandle NH;
    ros::MultiThreadedSpinner MTS;

    //=====Parameter
    NH.getParam("rss/check_url", rss_check_url);
    NH.getParam("rss/store_url", rss_store_url);
    NH.getParam("rss/jm_code", rss_jm_code);
    NH.getParam("tipe_control_unit", tipe_control_unit);
    NH.getParam("tid", tid);
    NH.getParam("mid", mid);
    //=====ServiceServer
    ser_rss_check = NH.advertiseService("rss/check", cllbck_ser_rss_check);
    ser_rss_store = NH.advertiseService("rss/store", cllbck_ser_rss_store);
    //=====Help
    help.init(&NH);

    if (rss_init() == -1)
        ros::shutdown();

    MTS.spin();
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

bool cllbck_ser_rss_check(slff::rss_check::Request &req, slff::rss_check::Response &res)
{
    std::string response;
    char error[256];

    CURL *curl;
    CURLcode curlcode;

    if ((curl = curl_easy_init()) != NULL)
    {
        curl_mime *mime = curl_mime_init(curl);
        curl_mimepart *mimepart;

        // JM Code
        ADD_FORM(mime, mimepart, "jm_code", rss_jm_code.c_str());
        // TID dan MID
        ADD_FORM(mime, mimepart, "tid", tid.c_str());
        ADD_FORM(mime, mimepart, "mid", mid.c_str());
        // RFID TID
        ADD_FORM(mime, mimepart, "rfid", req.rfid_tid.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, rss_check_url.c_str());
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);

        curlcode = curl_easy_perform(curl);

        curl_mime_free(mime);
        curl_easy_cleanup(curl);
    }

    if (curlcode == CURLE_OK)
    {
        res.success = true;
        res.response = response;
    }
    else
    {
        res.success = false;
        res.response = std::string(error);
    }

    return true;
}

bool cllbck_ser_rss_store(slff::rss_store::Request &req, slff::rss_store::Response &res)
{
    std::string response;
    char error[256];

    CURL *curl;
    CURLcode curlcode;

    if ((curl = curl_easy_init()) != NULL)
    {
        curl_mime *mime = curl_mime_init(curl);
        curl_mimepart *mimepart;

        char buffer[1024];

        // JM Code
        ADD_FORM(mime, mimepart, "jm_code", rss_jm_code.c_str());
        // TID dan MID
        ADD_FORM(mime, mimepart, "tid", tid.c_str());
        ADD_FORM(mime, mimepart, "mid", mid.c_str());
        // RFID flag
        ADD_FORM(mime, mimepart, "status_code", std::to_string(req.rfid_flag).c_str());
        if (req.metode_pembayaran == 3)
            ADD_FORM(mime, mimepart, "status_deteksi", "0")
        else
            ADD_FORM(mime, mimepart, "status_deteksi", "1");
        // RFID TID
        ADD_FORM(mime, mimepart, "rfid", req.rfid_tid.c_str());
        // No gardu, no gerbang, DateTime entrance dan DateTime exit
        if (tipe_control_unit == TIPE_OPEN ||
            tipe_control_unit == TIPE_EXIT ||
            tipe_control_unit == TIPE_EXIT_OPEN)
        {
            ADD_FORM(mime, mimepart, "gardu_id", std::to_string(req.no_gardu_exit).c_str());
            ADD_FORM(mime, mimepart, "gerbang_id", std::to_string(req.no_gerbang_exit).c_str());
            sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", req.exit_year + 2000, req.exit_month, req.exit_day, req.exit_hour, req.exit_minute, req.exit_second);
            ADD_FORM(mime, mimepart, "gto_timestamp", buffer);
        }
        else if (tipe_control_unit == TIPE_ENTRANCE ||
                 tipe_control_unit == TIPE_OPEN_ENTRANCE)
        {
            ADD_FORM(mime, mimepart, "gardu_id", std::to_string(req.no_gardu_entrance).c_str());
            ADD_FORM(mime, mimepart, "gerbang_id", std::to_string(req.no_gerbang_entrance).c_str());
            sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", req.entrance_year + 2000, req.entrance_month, req.entrance_day, req.entrance_hour, req.entrance_minute, req.entrance_second);
            ADD_FORM(mime, mimepart, "gto_timestamp", buffer);
        }
        // DateTime transaksi
        sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", req.transaction_year + 2000, req.transaction_month, req.transaction_day, req.transaction_hour, req.transaction_minute, req.transaction_second);
        ADD_FORM(mime, mimepart, "trans_date", buffer);
        // Date laporan
        sprintf(buffer, "%04d-%02d-%02d", req.report_year + 2000, req.report_month, req.report_day);
        ADD_FORM(mime, mimepart, "tgl_laporan", buffer);
        // Kode ruas
        ADD_FORM(mime, mimepart, "ruas", std::to_string(req.kode_ruas).c_str());
        // No shift, perioda, resi, KSPT, dan PLT
        ADD_FORM(mime, mimepart, "shift", std::to_string(req.no_shift).c_str());
        ADD_FORM(mime, mimepart, "perioda", std::to_string(req.no_perioda).c_str());
        ADD_FORM(mime, mimepart, "no_resi", std::to_string(req.no_resi).c_str());
        ADD_FORM(mime, mimepart, "kspt", std::to_string(req.no_kspt).c_str());
        ADD_FORM(mime, mimepart, "pultol", std::to_string(req.no_plt).c_str());
        // Hash
        ADD_FORM(mime, mimepart, "hash", req.hash.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, rss_store_url.c_str());
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);

        curlcode = curl_easy_perform(curl);

        curl_mime_free(mime);
        curl_easy_cleanup(curl);
    }

    if (curlcode == CURLE_OK)
    {
        res.success = true;
        res.response = response;
    }
    else
    {
        res.success = false;
        res.response = std::string(error);
    }

    return true;
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

int rss_init()
{
    return 0;
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    ((std::string *)userdata)->append((char *)ptr, size * nmemb);
    return size * nmemb;
}