/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <string.h>
#include <esp_log.h>
#include <ds3231.h>
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <time.h>
#include <esp_err.h>

#define TAG "mains"

#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_SDA_IO           GPIO_NUM_39
#define I2C_MASTER_SCL_IO           GPIO_NUM_38
#define I2C_MASTER_FREQ_HZ          400000


static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}

void app_main(void)
{
   ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_init());
   struct tm time={0};
   /**
    * @brief setting new time
    *
    */

   time.tm_yday = 2022-1900;
   time.tm_mon = 5-1;
   time.tm_mday = 9;
   time.tm_hour = 1;
   time.tm_min = 23;

   ESP_ERROR_CHECK_WITHOUT_ABORT(ds3231_set_time(I2C_MASTER_NUM,&time));
   vTaskDelay(pdMS_TO_TICKS(2*1000));
   ESP_ERROR_CHECK_WITHOUT_ABORT(ds3231_get_time(I2C_MASTER_NUM,&time));
   vTaskDelay(pdMS_TO_TICKS(60*1000));
   printf("%d\n",time.tm_hour);


}
