#ifndef MISC_H_
#define MISC_H_

#include "boost/algorithm/hex.hpp"
#include "boost/date_time.hpp"
#include "curl/curl.h"
#include "ros/ros.h"
#include "slff/buzzer.h"
#include "slff/log.h"
#include "slff/rs232_baudrate.h"
#include "slff/rs232_data.h"
#include "std_msgs/UInt16.h"
#include "std_msgs/UInt8.h"
#include "json/json.hpp"

#define JSON_READ(src, dst) \
    if (!src.is_null()) dst = src

#define JSON_READ_ATOI(src, dst) \
    if (!src.is_null() && std::string(src).length() > 0) dst = std::stoi(std::string(src))

#define JSON_READ_ITOA(src, dst) \
    if (!src.is_null()) dst = std::to_string(src)

#define JSON_WRITE(dst, src) \
    dst = src

#define ADD_FORM(mime, mimepart, name, data)                  \
    {                                                         \
        mimepart = curl_mime_addpart(mime);                   \
        curl_mime_name(mimepart, name);                       \
        curl_mime_data(mimepart, data, CURL_ZERO_TERMINATED); \
    }

#define MEMCPY(dst, src, len)  \
    {                          \
        memset(dst, 0, len);   \
        memcpy(dst, src, len); \
    }

#define STRCPY(dst, src, len)          \
    {                                  \
        memset(dst, 0, len);           \
        src.copy((char *)dst, len, 0); \
    }

#define STRCPY2(dst, src, len)               \
    {                                        \
        dst = std::string((char *)src, len); \
    }

class misc
{
  private:
    ros::Publisher pub_log;
    ros::Publisher pub_opto_out;
    ros::Publisher pub_led_out;
    ros::Publisher pub_buzzer;
    ros::Publisher pub_rs232_baudrate;
    ros::Publisher pub_rs232_tx;

    uint8_t opto_out_data;
    uint16_t led_out_data;

    void log_help(const char *header, const char *format, va_list args);

  public:
    misc();
    ~misc();

    void init(ros::NodeHandle *NH);

    void log_info(const char *format, ...);
    void log_warn(const char *format, ...);
    void log_error(const char *format, ...);
    void log_fatal(const char *format, ...);

    void opto_out(uint8_t data);
    void opto_out(uint8_t channel, bool data);

    void led_out(uint16_t data);
    void led_out(uint8_t channel, bool data);

    void buzzer(uint16_t waktu, uint16_t jumlah);

    void rs232_baudrate(uint8_t channel, uint32_t baudrate);
    void rs232_tx(uint8_t channel, std::vector<uint8_t> data);
};

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

misc::misc()
{
}

misc::~misc()
{
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void misc::init(ros::NodeHandle *NH)
{
    pub_log = NH->advertise<slff::log>("log", 0);
    pub_opto_out = NH->advertise<std_msgs::UInt8>("expansion/opto_out", 0);
    pub_led_out = NH->advertise<std_msgs::UInt16>("expansion/led_out", 0);
    pub_buzzer = NH->advertise<slff::buzzer>("expansion/buzzer", 0);
    pub_rs232_baudrate = NH->advertise<slff::rs232_baudrate>("expansion/rs232_baudrate", 0);
    pub_rs232_tx = NH->advertise<slff::rs232_data>("expansion/rs232_tx", 0);
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void misc::log_help(const char *header, const char *format, va_list args)
{
    char message[1024];
    vsnprintf(message, sizeof(message), format, args);

    boost::posix_time::ptime time_ptime = boost::posix_time::microsec_clock::local_time();
    std::string time_string = boost::posix_time::to_simple_string(time_ptime);

    slff::log msg_log;
    msg_log.datetime = time_string.substr(12, 12);
    msg_log.header = std::string(header);
    msg_log.message = std::string(message);
    pub_log.publish(msg_log);
}

void misc::log_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_help(" INFO", format, args);
    va_end(args);
}

void misc::log_warn(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_help(" WARN", format, args);
    va_end(args);
}

void misc::log_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_help("ERROR", format, args);
    va_end(args);
}

void misc::log_fatal(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_help("FATAL", format, args);
    va_end(args);
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void misc::opto_out(uint8_t data)
{
    opto_out_data = data;

    std_msgs::UInt8 msg_opto_out;
    msg_opto_out.data = opto_out_data;
    pub_opto_out.publish(msg_opto_out);
}

void misc::opto_out(uint8_t channel, bool data)
{
    if (data)
        opto_out_data |= 1 << channel;
    else
        opto_out_data &= ~(1 << channel);

    std_msgs::UInt8 msg_opto_out;
    msg_opto_out.data = opto_out_data;
    pub_opto_out.publish(msg_opto_out);
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void misc::led_out(uint16_t data)
{
    led_out_data = data;

    std_msgs::UInt16 msg_led_out;
    msg_led_out.data = led_out_data;
    pub_led_out.publish(msg_led_out);
}

void misc::led_out(uint8_t channel, bool data)
{
    if (data)
        led_out_data |= 1 << channel;
    else
        led_out_data &= ~(1 << channel);

    std_msgs::UInt16 msg_led_out;
    msg_led_out.data = led_out_data;
    pub_led_out.publish(msg_led_out);
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void misc::buzzer(uint16_t waktu, uint16_t jumlah)
{
    slff::buzzer msg_buzzer;
    msg_buzzer.waktu = waktu;
    msg_buzzer.jumlah = jumlah;
    pub_buzzer.publish(msg_buzzer);
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void misc::rs232_baudrate(uint8_t channel, uint32_t baudrate)
{
    slff::rs232_baudrate msg_rs232_baudrate;
    msg_rs232_baudrate.channel = channel;
    msg_rs232_baudrate.baudrate = baudrate;
    pub_rs232_baudrate.publish(msg_rs232_baudrate);
}

void misc::rs232_tx(uint8_t channel, std::vector<uint8_t> data)
{
    slff::rs232_data msg_rs232_tx;
    msg_rs232_tx.channel = channel;
    msg_rs232_tx.data = data;
    pub_rs232_tx.publish(msg_rs232_tx);
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

void byte_array_to_hex_string(std::string &str, uint8_t *data, uint8_t size)
{
    str = boost::algorithm::hex(std::string((char *)data, size));
}

void hex_string_to_byte_array(std::string &str, uint8_t *data, uint8_t size)
{
    memcpy(data, boost::algorithm::unhex(str).data(), size);
}

//=============================================================================
//-----------------------------------------------------------------------------
//=============================================================================

nlohmann::json json_read(std::string path)
{
    nlohmann::json j;

    std::ifstream file(path.c_str());
    file >> j;
    file.close();

    return j;
}

void json_write(std::string path, nlohmann::json j)
{
    std::ofstream f(path);
    f << j.dump(4);
    f.close();
}

#endif