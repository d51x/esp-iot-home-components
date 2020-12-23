#pragma once

#ifndef _I2C_BUS_H_
#define _I2C_BUS_H_
#include "driver/i2c.h"
#include "nvsparam.h"
#include "freertos/semphr.h"

#ifdef CONFIG_COMPONENT_I2C

#define WRITE_BIT                           I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                            I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                        0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                       0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                             0x0              /*!< I2C ack value */
#define NACK_VAL                            0x1              /*!< I2C nack value */
#define LAST_NACK_VAL                       0x2              /*!< I2C last_nack value */

#define i2c_master_wait    os_delay_us

#define I2C_SEMAPHORE_WAIT_TIME_MS             100
#define I2C_SEMAPHORE_WAIT  (I2C_SEMAPHORE_WAIT_TIME_MS /  portTICK_RATE_MS)

#define I2C_SDA_DEFAULT 2
#define I2C_SCL_DEFAULT 0

extern const char *PARAM_I2C ICACHE_RODATA_ATTR;
extern const char *PARAM_I2C_SDA ICACHE_RODATA_ATTR;
extern const char *PARAM_I2C_SCL ICACHE_RODATA_ATTR;

typedef void* i2c_bus_handle_t;

SemaphoreHandle_t xSemaphoreI2C;

/**
 * @brief Create and init I2C bus and return a I2C bus handle
 *
 * @param port I2C port number
 * @param conf Pointer to I2C parameters
 *
 * @return
 *     - NULL Fail
 *     - Others Success
 */
i2c_bus_handle_t i2c_bus_create(i2c_port_t port, i2c_config_t* conf);

/**
 * @brief Delete and release the I2C bus object
 *
 * @param bus I2C bus handle
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t i2c_bus_delete(i2c_bus_handle_t bus);

/**
 * @brief I2C start sending buffered commands
 *
 * @param bus I2C bus handle
 * @param cmd I2C cmd handle
 * @param ticks_to_wait Maximum blocking time
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t i2c_bus_cmd_begin(i2c_bus_handle_t bus, i2c_cmd_handle_t cmd,
portBASE_TYPE ticks_to_wait);
//#ifdef __cplusplus
//}
//#endif

#ifdef CONFIG_COMPONENT_I2C_SCANNER
uint8_t i2c_bus_scan(i2c_bus_handle_t bus, uint8_t* devices);
#endif

i2c_bus_handle_t i2c_bus_init();

void i2c_load_cfg(i2c_config_t *cfg);
void i2c_save_cfg(const i2c_config_t *cfg);

esp_err_t i2c_device_available(uint8_t addr);
esp_err_t i2c_send_command(uint8_t addr, uint8_t cmd);
esp_err_t i2c_write_data(uint8_t addr, uint8_t *data, size_t sz);
esp_err_t i2c_read_data(uint8_t addr, uint8_t *data, size_t sz);
#endif //#ifdef CONFIG_COMPONENT_I2C

#endif