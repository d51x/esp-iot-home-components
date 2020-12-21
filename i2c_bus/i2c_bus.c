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

#ifdef CONFIG_COMPONENT_I2C

const char *PARAM_I2C = "i2c";
const char *PARAM_I2C_SDA = "sda";
const char *PARAM_I2C_SCL = "scl";


typedef struct {
    i2c_config_t i2c_conf;   /*!<I2C bus parameters*/
    i2c_port_t i2c_port;     /*!<I2C port number */
} i2c_bus_t;

static const char* TAG = "i2c_bus";
#define I2C_BUS_CHECK(a, str, ret)  if(!(a)) {                                             \
    ESP_LOGE(TAG,"%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str);      \
    return (ret);                                                                   \
    }
#define ESP_INTR_FLG_DEFAULT  (0)
#define ESP_I2C_MASTER_BUF_LEN  (0)

static i2c_bus_handle_t *m_i2c_bus_handle = NULL;

void i2c_load_cfg(i2c_config_t *cfg)
{
    if ( nvs_param_u8_load(PARAM_I2C, PARAM_I2C_SDA, &cfg->sda_io_num) != ESP_OK ) {
        cfg->sda_io_num = I2C_SDA_DEFAULT;
    }
    if ( cfg->sda_io_num > 16 )
        cfg->sda_io_num = I2C_SDA_DEFAULT;

    if ( nvs_param_u8_load(PARAM_I2C, PARAM_I2C_SCL, &cfg->scl_io_num) != ESP_OK ) {
        cfg->scl_io_num = I2C_SCL_DEFAULT;
    }
    if ( cfg->scl_io_num > 16 )
        cfg->scl_io_num = I2C_SCL_DEFAULT;    
}

void i2c_save_cfg(const i2c_config_t *cfg)
{
    nvs_param_u8_save(PARAM_I2C, PARAM_I2C_SDA, cfg->sda_io_num > 16 ? I2C_SDA_DEFAULT : cfg->sda_io_num);
    nvs_param_u8_save(PARAM_I2C, PARAM_I2C_SCL, cfg->scl_io_num > 16 ? I2C_SCL_DEFAULT : cfg->scl_io_num);
}

i2c_bus_handle_t i2c_bus_create(i2c_port_t port, i2c_config_t* conf)
{
    
    I2C_BUS_CHECK(port < I2C_NUM_MAX, "I2C port error", NULL);
    I2C_BUS_CHECK(conf != NULL, "Pointer error", NULL);

    i2c_bus_t* bus = (i2c_bus_t*) calloc(1, sizeof(i2c_bus_t));
    bus->i2c_conf = *conf;
    bus->i2c_port = port;

    esp_err_t ret   = ESP_FAIL;
    //ret = i2c_driver_install(bus->i2c_port, bus->i2c_conf.mode, ESP_I2C_MASTER_BUF_LEN, ESP_I2C_MASTER_BUF_LEN, ESP_INTR_FLG_DEFAULT);
    ret = i2c_driver_install(bus->i2c_port, bus->i2c_conf.mode);
    if(ret != ESP_OK) {
        goto error;
    }

    ret = i2c_param_config(bus->i2c_port, &bus->i2c_conf);
    if(ret != ESP_OK) {
        goto error;
    }

    return (i2c_bus_handle_t) bus;

    error:
    if(bus) {
        free(bus);
    }
    return NULL;
}

esp_err_t i2c_bus_delete(i2c_bus_handle_t bus)
{
    I2C_BUS_CHECK(bus != NULL, "Handle error", ESP_FAIL);
    i2c_bus_t* i2c_bus = (i2c_bus_t*) bus;
    i2c_driver_delete(i2c_bus->i2c_port);
    free(bus);
    return ESP_OK;
}

esp_err_t i2c_bus_cmd_begin(i2c_bus_handle_t bus, i2c_cmd_handle_t cmd, portBASE_TYPE ticks_to_wait)
{
    I2C_BUS_CHECK(bus != NULL, "Handle error", ESP_FAIL);
    I2C_BUS_CHECK(cmd != NULL, "I2C cmd error", ESP_FAIL);
    i2c_bus_t* i2c_bus = (i2c_bus_t*) bus;
    return i2c_master_cmd_begin(i2c_bus->i2c_port, cmd, ticks_to_wait);
}

i2c_bus_handle_t i2c_bus_init()
{
    if ( m_i2c_bus_handle ) return m_i2c_bus_handle; // уже инициализировано
    
    if ( xSemaphoreI2C != NULL ) vSemaphoreDelete( xSemaphoreI2C );
    xSemaphoreI2C = xSemaphoreCreateMutex();

    i2c_config_t conf;
    i2c_load_cfg( &conf );

    conf.mode = I2C_MODE_MASTER;
    //conf.scl_io_num = 0;
    //conf.sda_io_num = 2;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    //conf.master.clk_speed = clk_hz;
    conf.clk_stretch_tick = 300;

    //i2c_bus_handle_t m = i2c_bus_create(I2C_NUM_0, &conf);
    m_i2c_bus_handle = i2c_bus_create(I2C_NUM_0, &conf);

    return m_i2c_bus_handle;
}

uint8_t i2c_bus_scan(i2c_bus_handle_t bus, uint8_t* devices)
{
    
    if( xSemaphoreI2C == NULL ) return 0;
    uint8_t devices_found = 0;

    if( xSemaphoreTake( xSemaphoreI2C, I2C_SEMAPHORE_WAIT ) == pdTRUE )
    {
        
        for (uint8_t address = 1; address < 127; address++) {
            if ( i2c_device_available(address) == ESP_OK )
            {
                devices[devices_found] = address;
                devices_found++;            
            }
        }
        xSemaphoreGive( xSemaphoreI2C );
        
    }
    //else
    //{
        //ESP_LOGE(TAG, "%s: semaphore locked!", __func__ );
    //}
    
    return devices_found;
}

esp_err_t i2c_device_available(uint8_t addr)
{
    i2c_cmd_handle_t cmd_link = i2c_cmd_link_create();
    ESP_ERROR_CHECK( i2c_master_start(cmd_link) );
    ESP_ERROR_CHECK( i2c_master_write_byte(cmd_link, (addr << 1) | WRITE_BIT, ACK_CHECK_EN) );
    ESP_ERROR_CHECK( i2c_master_stop(cmd_link) );
	esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd_link, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd_link); 
    return err;
}

esp_err_t i2c_send_command(uint8_t addr, uint8_t command)
{
    
    return i2c_write_data( addr, &command, 1);
    
    // Wire.begin / Wire.beginTransmission
    //i2c_cmd_handle_t cmd_link = i2c_cmd_link_create();
    //ESP_ERROR_CHECK( i2c_master_start(cmd_link) );
    //ESP_ERROR_CHECK( i2c_master_write_byte(cmd_link, (addr << 1) | WRITE_BIT, ACK_CHECK_EN));
    
    // write command 
    //ESP_ERROR_CHECK( i2c_master_write_byte(cmd_link, command, ACK_CHECK_EN) );
    //ESP_ERROR_CHECK( i2c_master_stop(cmd_link)); 
    //esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd_link, /*0 ?*/ 1000 / portTICK_RATE_MS) ;
    //i2c_cmd_link_delete(cmd_link); 
   // return err;
}

esp_err_t i2c_write_data(uint8_t addr, uint8_t *data, size_t sz)
{
    i2c_cmd_handle_t cmd_link = i2c_cmd_link_create();
    ESP_ERROR_CHECK( i2c_master_start(cmd_link) );
    ESP_ERROR_CHECK( i2c_master_write_byte(cmd_link, (addr << 1) | WRITE_BIT, ACK_CHECK_EN));

    // write data
    for ( uint8_t i = 0; i < sz; i++ )
    {
        ESP_ERROR_CHECK( i2c_master_write_byte(cmd_link, data[i], ACK_CHECK_EN) );
    }

    ESP_ERROR_CHECK( i2c_master_stop(cmd_link)); 
    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd_link, /*0 ?*/ 1000 / portTICK_RATE_MS) ;
    i2c_cmd_link_delete(cmd_link); 
    return err;
}

esp_err_t i2c_read_data(uint8_t addr, uint8_t *data, size_t sz)
{
    // Wire.begin / Wire.beginTransmission
    i2c_cmd_handle_t cmd_link = i2c_cmd_link_create();
    ESP_ERROR_CHECK( i2c_master_start(cmd_link) );
    ESP_ERROR_CHECK( i2c_master_write_byte(cmd_link, (addr << 1) | READ_BIT, ACK_CHECK_EN) ); 

    //if ( err == ESP_FAIL ) return err;

    // read data
    for ( uint8_t i = 0; i < sz; i++ )
    {
        ESP_ERROR_CHECK( i2c_master_read_byte(cmd_link, &data[i], ( i < sz-1 ) ? ACK_VAL : NACK_VAL) );
    }

    ESP_ERROR_CHECK( i2c_master_stop(cmd_link) );
    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd_link, 1000 / portTICK_RATE_MS) ;
    i2c_cmd_link_delete(cmd_link); 

    return err;
}


#endif //#ifdef CONFIG_COMPONENT_I2C