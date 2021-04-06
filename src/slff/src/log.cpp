#include "slff/log.h"
#include "boost/filesystem.hpp"
#include "ros/ros.h"
#include "slff/color.h"
#include "slff/define.h"
#include "slff/misc.h"

//=====Prototype
void cllbck_sub_log(const slff::logConstPtr &msg);

int log_init();

std::string folder_to_make();
std::string folder_to_remove();
void make_folder(std::string path);
void remove_folder(std::string path);

//=====Subscriber
ros::Subscriber sub_log;

//=====Log
std::string log_path;
std::ofstream log_file;

int main(int argc, char **argv)
{
    ros::init(argc, argv, "log");

    ros::NodeHandle NH;
    ros::MultiThreadedSpinner MTS;

    //=====Subscriber
    sub_log = NH.subscribe("log", 0, cllbck_sub_log);

    if (log_init() == -1)
        ros::shutdown();

    MTS.spin();
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void cllbck_sub_log(const slff::logConstPtr &msg)
{
    make_folder(folder_to_make());
    remove_folder(folder_to_remove());

    time_t _time = time(NULL);
    struct tm _tm = *localtime(&_time);

    char buffer[64];
    sprintf(buffer, "slff-%04d-%02d-%02d.txt", _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday);
    std::string path = folder_to_make() + "/" + std::string(buffer);

    std::string log_header = color::rize(msg->header, "Blue");
    std::string log_datetime = color::rize(msg->datetime, "Green");

    std::string log_message;
    if (msg->header.find("INFO") != std::string::npos)
        log_message = color::rize(msg->message, "White");
    else if (msg->header.find("WARN") != std::string::npos)
        log_message = color::rize(msg->message, "Yellow");
    else if (msg->header.find("ERROR") != std::string::npos ||
             msg->header.find("FATAL") != std::string::npos)
        log_message = color::rize(msg->message, "Red");
    else
        log_message = color::rize(msg->message, "White");

    std::cout << log_datetime << " [" << log_header << "] " << log_message << std::endl;

    log_file.open(path.c_str(), std::ofstream::out | std::ofstream::app);
    log_file << msg->datetime << " [" << msg->header << "] " << msg->message << std::endl;
    log_file.close();
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

int log_init()
{
    log_path = getenv("HOME") + std::string("/slff-data/log/");

    if (!boost::filesystem::exists(log_path))
        boost::filesystem::create_directories(log_path);

    return 0;
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

std::string folder_to_make()
{
    time_t _time = time(NULL);
    struct tm _tm = *localtime(&_time);

    int year = _tm.tm_year + 1900;
    int month = _tm.tm_mon + 1;

    char buffer[64];
    sprintf(buffer, "slff-%04d-%02d", year, month);
    std::string path = log_path + std::string(buffer);

    return path;
}

std::string folder_to_remove()
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
    sprintf(buffer, "slff-%04d-%02d", year, month);
    std::string path = log_path + std::string(buffer);

    return path;
}

void make_folder(std::string path)
{
    if (!boost::filesystem::exists(path))
        boost::filesystem::create_directories(path);
}

void remove_folder(std::string path)
{
    if (boost::filesystem::exists(path))
        boost::filesystem::remove_all(path);
}