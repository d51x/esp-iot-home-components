// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.



#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "i2c_bus.h"
#include "sht21.h"

#ifdef CONFIG_SENSORS_GET
#include "sensors.h"
#endif

#ifdef CONFIG_SENSOR_SHT21


static i2c_bus_handle_t sht21_i2c_bus_handle = NULL;

static const char* TAG = "SHT21";

/*
// HOLD MASTER - SCL line is blocked (controlled by sensor) during measurement
// NO HOLD MASTER - allows other I2C communication tasks while sensor performing
// measurements.
*/
#define SHT21_POLYNOMIAL 0x13100  // P(x)=x^8+x^5+x^4+1 = 100110001

#define SHT21_CMD_TRIGGER_TEMP_MEASURE_HOLD 0xE3  
#define SHT21_CMD_TRIGGER_TEMP_MEASURE_NO_HOLD 0xF3

#define SHT21_CMD_TRIGGER_HUM_MEASURE_HOLD 0xE5  
#define SHT21_CMD_TRIGGER_HUM_MEASURE_NO_HOLD 0xF5  

#define SHT21_CMD_SOFT_RESET 0xFE
#define SHT21_READ_ERROR 65535

#define SHT21_PERIODIC_TASK_PRIORITY 13
#define SHT21_PERIODIC_TASK_STACK_DEPTH 1024

static volatile sht21_t sht21_data;
TaskHandle_t xHandle = NULL;
uint8_t is_initialized = 0;

static void sht21_mqtt_send();


static esp_err_t sht21_reset()
{
    // Wire.begin / Wire.beginTransmission
    /*
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHT21_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);

    i2c_master_write_byte(cmd, SHT21_CMD_SOFT_RESET, ACK_CHECK_EN);
    i2c_master_stop(cmd); 
    err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);  
    */
    return i2c_send_command(SHT21_ADDR, SHT21_CMD_SOFT_RESET);
}



static uint8_t sht21_check_crc(uint16_t data)
{
  for (uint8_t bit = 0; bit < 16; bit++)
  {
    if   (data & 0x8000) data = (data << 1) ^ SHT21_POLYNOMIAL;
    else data <<= 1;
  }
  return data >>= 8;
}

esp_err_t sht21_available()
{
    return i2c_device_available(SHT21_ADDR);
}



esp_err_t sht21_init()
{
    esp_err_t err = ESP_FAIL;

    sht21_i2c_bus_handle = i2c_bus_init();
    
    if ( sht21_i2c_bus_handle == NULL ) return err;

    err = sht21_available();
    is_initialized = ( err == ESP_OK );

    return err;
}


static uint16_t sht21_read_raw_data(uint8_t command) 
{
    if ( xSemaphoreI2C == NULL ) return SHT21_READ_ERROR;

    if( xSemaphoreTake( xSemaphoreI2C, I2C_SEMAPHORE_WAIT ) == pdTRUE )
    {
        esp_err_t err = i2c_send_command(SHT21_ADDR, command);
        if ( err == ESP_FAIL ) goto error;

        vTaskDelay( 100 / portTICK_RATE_MS);

        uint8_t data[3];
        err = i2c_read_data(SHT21_ADDR, &data, 3);
        if ( err == ESP_FAIL ) goto error;
        
        uint16_t res = (data[0] << 8);
        res += data[1];

        //ESP_LOGI(TAG, "Cmd: 0x%2X\tdata0: 0x%02X\t data1: 0x%02X\t data2: 0x%02X\t", command, data[0], data[1], data[2]);
        // check crc
        uint8_t crc = sht21_check_crc( res );
        if (  crc != data[2] ) goto error;

        xSemaphoreGive( xSemaphoreI2C );

        return res;

        error:
            xSemaphoreGive( xSemaphoreI2C );
            return SHT21_READ_ERROR; 

    } else {
        //ESP_LOGI(TAG, "sht21_read error. xSemaphoreI2C blocked!!!");
        return SHT21_READ_ERROR; 
    }

}

float sht21_get_temp()
{
    return sht21_data.temp;
}

float sht21_get_hum()
{
    return sht21_data.hum;
}

float sht21_read_temp()
{
    float res = 255.0f;

    uint16_t raw_data = sht21_read_raw_data(SHT21_CMD_TRIGGER_TEMP_MEASURE_NO_HOLD);
    if ( raw_data == SHT21_READ_ERROR ) return res;
    res = (0.002681 * (float) raw_data - 46.85);

    return res;
}

float sht21_read_hum()
{
    float res = 0.0f;

    uint16_t raw_data = sht21_read_raw_data(SHT21_CMD_TRIGGER_HUM_MEASURE_NO_HOLD);
    if ( raw_data == SHT21_READ_ERROR ) return res;
    
    raw_data ^= 0x02;
    res = (0.001907 * (float)raw_data - 6);

    if      (res < 0)   res = 0.0f;                       
    else if (res > 100) res = 100.0f;

    return res;
}

static void sht21_periodic_task(void *arg)
{
    uint32_t delay = (uint32_t)arg;

    while (1) {
        //ESP_LOGI(TAG, "%s is_initialized %d", __func__, is_initialized);
        if ( !is_initialized ) 
        {
            sht21_init();
            vTaskDelay( 5000 / portTICK_RATE_MS);
            continue;
        } 

        float t;
        t = sht21_read_temp();
        if ( t < 255 ) sht21_data.temp = t;
        
        sht21_data.hum = sht21_read_hum();
        vTaskDelay(delay * 1000 / portTICK_RATE_MS);
    }

    vTaskDelete( NULL );
}


#ifdef CONFIG_SENSORS_GET
static void sht21_sensors_print(char **buf, void *args)
{
    size_t sz = get_buf_size("shtt:%0.2f;shth:%0.2f;", sht21_data.temp, sht21_data.hum);
    *buf = (char *) realloc(*buf, sz+1);
    memset(*buf, 0, sz+1);
    snprintf(*buf, sz+1, "shtt:%0.2f;shth:%0.2f;", sht21_data.temp, sht21_data.hum);
}
#endif

void sht21_start(uint32_t delay)
{
    xTaskCreate(sht21_periodic_task, "sht21_task", SHT21_PERIODIC_TASK_STACK_DEPTH, delay, SHT21_PERIODIC_TASK_PRIORITY, &xHandle);

    #ifdef CONFIG_SENSORS_GET
    sensors_add("sht21", sht21_sensors_print, NULL); 
    #endif
}


void sht21_stop()
{
     if( xHandle != NULL )
     {
         vTaskDelete( xHandle );
     }
}


#endif
