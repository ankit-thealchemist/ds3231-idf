/**
 * @file ds3231.c
 * @author Ankit Bansal (iotdevelope@gmail.com)
 * @brief Implementation of ds3231s
 * @version 1.1
 * @date 2022-11-12
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <esp_err.h>
#include "ds3231.h"
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/task.h>
#include <string.h>
#include <esp_log.h>

#define I2C_FREQ_HZ 400000

#define DS3231_STAT_OSCILLATOR 0x80
#define DS3231_STAT_32KHZ 0x08
#define DS3231_STAT_ALARM_2 0x02
#define DS3231_STAT_ALARM_1 0x01

#define DS3231_CTRL_OSCILLATOR 0x80
#define DS3231_CTRL_TEMPCONV 0x20
#define DS3231_CTRL_ALARM_INTS 0x04
#define DS3231_CTRL_ALARM2_INT 0x02
#define DS3231_CTRL_ALARM1_INT 0x01

#define DS3231_ALARM_WDAY 0x40
#define DS3231_ALARM_NOTSET 0x80

#define DS3231_ADDR_TIME 0x00
#define DS3231_ADDR_ALARM1 0x07
#define DS3231_ADDR_ALARM2 0x0b
#define DS3231_ADDR_CONTROL 0x0e
#define DS3231_ADDR_STATUS 0x0f
#define DS3231_ADDR_AGING 0x10
#define DS3231_ADDR_TEMP 0x11

#define DS3231_12HOUR_FLAG 0x40
#define DS3231_12HOUR_MASK 0x1f
#define DS3231_PM_FLAG 0x20
#define DS3231_MONTH_MASK 0x1f

#define TAG "ds3231"

enum
{
    DS3231_SET = 0,
    DS3231_CLEAR,
    DS3231_REPLACE
};

static uint8_t bcd2dec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0f);
}

static uint8_t dec2bcd(uint8_t val)
{
    return ((val / 10) << 4) + (val % 10);
}

esp_err_t ds3231_set_time(i2c_port_t port, struct tm *time)
{

    uint8_t data[7] = {0};
    uint8_t write_buf[8] = {0};

    /* time/date data */
    data[0] = dec2bcd(time->tm_sec);
    data[1] = dec2bcd(time->tm_min);
    data[2] = dec2bcd(time->tm_hour);
    /* The week data must be in the range 1 to 7, and to keep the start on the
     * same day as for tm_wday have it start at 1 on Sunday. */
    data[3] = dec2bcd(time->tm_wday + 1);
    data[4] = dec2bcd(time->tm_mday);
    data[5] = dec2bcd(time->tm_mon + 1);
    data[6] = dec2bcd(time->tm_year - 100);

    write_buf[0] = DS3231_ADDR_TIME;
    memcpy(write_buf + 1, data, sizeof(data));

    ESP_LOGD(TAG,"Sending buffer to ds3231");
    ESP_LOG_BUFFER_HEX_LEVEL(TAG,write_buf,sizeof(write_buf),ESP_LOG_DEBUG);

    return i2c_master_write_to_device(port, DS3231_ADDR, write_buf, sizeof(write_buf), DEFAULT_TIMEOUT_MS / portTICK_PERIOD_MS);
}

esp_err_t ds3231_get_time(i2c_port_t port, struct tm *time)
{

    uint8_t data[7] = {0};
    uint8_t write_reg = DS3231_ADDR_TIME;
    ESP_LOGD(TAG,"Writing to the register %02x",write_reg);
    /* read time */
    // I2C_DEV_CHECK(dev, i2c_dev_read_reg(dev, DS3231_ADDR_TIME, data, 7));

    esp_err_t err = i2c_master_write_read_device(port, DS3231_ADDR, &write_reg, 1, data, 7, DEFAULT_TIMEOUT_MS / portTICK_PERIOD_MS);

    ESP_LOGD(TAG,"Received data is ");
    ESP_LOG_BUFFER_HEX_LEVEL(TAG,data,sizeof(data),ESP_LOG_DEBUG);


    /* convert to unix time structure */
    time->tm_sec = bcd2dec(data[0]);
    time->tm_min = bcd2dec(data[1]);
    if (data[2] & DS3231_12HOUR_FLAG)
    {
        /* 12H */
        time->tm_hour = bcd2dec(data[2] & DS3231_12HOUR_MASK) - 1;
        /* AM/PM? */
        if (data[2] & DS3231_PM_FLAG)
            time->tm_hour += 12;
    }
    else
        time->tm_hour = bcd2dec(data[2]); /* 24H */
    time->tm_wday = bcd2dec(data[3]) - 1;
    time->tm_mday = bcd2dec(data[4]);
    time->tm_mon = bcd2dec(data[5] & DS3231_MONTH_MASK) - 1;
    time->tm_year = bcd2dec(data[6]) + 100;
    time->tm_isdst = 0;

    // apply a time zone (if you are not using localtime on the rtc or you want to check/apply DST)
    // applyTZ(time);

    return err;
}
