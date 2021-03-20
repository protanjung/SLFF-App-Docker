#include "boost/filesystem.hpp"
#include "boost/thread/mutex.hpp"
#include "ros/ros.h"
#include "slff/misc.h"
#include "sqlite3.h"

//=====Prototype
int database_init();

std::string file_to_make();
std::string file_to_remove();
void make_file(std::string path);
void remove_file(std::string path);

//=====Database
std::string database_path;
sqlite3 *database_file;

std::string gto_present_column[][2] =
    {{"no_seri_control_unit", "INTEGER PRIMARY KEY"},
     {"rfid", "TEXT"},
     {"golongan_kendaraan", "INTEGER"},
     {"jenis_kendaraan", "INTEGER"},
     {"plat_no_rss", "TEXT"},
     {"plat_no_anpr", "TEXT"},
     {"entrance_datetime", "TEXT"},
     {"no_gardu_entrance", "INTEGER"},
     {"no_gerbang_entrance", "INTEGER"},
     {"no_gardu_exit", "INTEGER"},
     {"no_gerbang_exit", "INTEGER"},
     {"mid", "TEXT"},
     {"tid", "TEXT"},
     {"saldo", "INTEGER"},
     {"tarif", "INTEGER"},
     {"hash", "TEXT"}};

std::string gto_store_column[][2] =
    {{"no_seri_control_unit", "INTEGER PRIMARY KEY"},
     {"rfid", "TEXT"},
     {"entrance_datetime", "TEXT"},
     {"exit_datetime", "TEXT"},
     {"report_date", "TEXT"},
     {"kode_ruas", "INTEGER"},
     {"no_shift", "INTEGER"},
     {"no_periode", "INTEGER"},
     {"no_resi", "INTEGER"},
     {"no_kspt", "INTEGER"},
     {"no_plt", "INTEGER"},
     {"hash", "TEXT"}};

size_t gto_present_column_size = sizeof(gto_present_column) / sizeof(gto_present_column[0]);
size_t gto_store_column_size = sizeof(gto_store_column) / sizeof(gto_store_column[0]);

int main(int argc, char **argv)
{
    ros::init(argc, argv, "database");

    ros::NodeHandle NH;
    ros::MultiThreadedSpinner MTS;

    if (database_init() == -1)
        ros::shutdown();

    MTS.spin();
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

int database_init()
{
    database_path = getenv("HOME") + std::string("/slff-data/database/");

    if (!boost::filesystem::exists(database_path))
        boost::filesystem::create_directories(database_path);

    return 0;
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

std::string file_to_make()
{
    time_t _time = time(NULL);
    struct tm _tm = *localtime(&_time);

    int year = _tm.tm_year + 1900;
    int month = _tm.tm_mon + 1;

    char buffer[64];
    sprintf(buffer, "slff %04d-%02d.db", year, month);
    std::string path = database_path + std::string(buffer);

    return path;
}

std::string file_to_remove()
{
    time_t _time = time(NULL);
    struct tm _tm = *localtime(&_time);

    int year = _tm.tm_year + 1900 - 1;
    int month = _tm.tm_mon + 1 - 6;

    while (month < 1)
    {
        year = year - 1;
        month = month + 12;
    }

    char buffer[64];
    sprintf(buffer, "slff %04d-%02d.db", year, month);
    std::string path = database_path + std::string(buffer);

    return path;
}

void make_file(std::string path)
{
    if (!boost::filesystem::exists(path))
    {
        std::ofstream file(path.c_str());
        file.close();
    }
}

void remove_file(std::string path)
{
    if (boost::filesystem::exists(path))
    {
        std::remove(path.c_str());
    }
}