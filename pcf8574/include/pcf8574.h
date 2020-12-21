#pragma once

#ifndef _PCF8574_H_
#define _PCF8574_H_

#include <stdio.h>
#include <stdlib.h>

#include "esp_err.h"
#include "driver/i2c.h"
#include "nvsparam.h"
#include "i2c_bus.h"
#include "freertos/task.h"

#include "utils.h"

#ifdef CONFIG_COMPONENT_PCF8574
#define PCF8574_ADDR_DEFAULT 0x20

typedef void (*pcf8574_isr_cb)(void *arg);

typedef void *pcf8574_handle_t; 

typedef enum pcf8574_status {
	PCF8574_DISABLED,
	PCF8574_ENABLED
} pcf8574_status_t;

typedef enum pcf8574_inverse {
	PCF8574_PORT_NORMAL,
	PCF8574_PORT_INVERTED
} pcf8574_inverse_t;

typedef struct pcf8574 {
	uint8_t addr;
	uint8_t input_value;
	uint8_t output_value;
	uint8_t gpio_num_isr;
	pcf8574_inverse_t inverse;
	pcf8574_status_t status;
	i2c_bus_handle_t i2c_bus_handle;
} pcf8574_t;




pcf8574_handle_t pcf8574_create(uint8_t addr);
esp_err_t pcf8574_enable(pcf8574_handle_t pcf8574_h);
esp_err_t pcf8574_disable(pcf8574_handle_t pcf8574_h);
esp_err_t pcf8574_delete(pcf8574_handle_t pcf8574_h);
esp_err_t pcf8574_available(pcf8574_handle_t pcf8574_h);

// читаем биты напрямую
esp_err_t pcf8574_read(pcf8574_handle_t pcf8574_h, uint8_t *value);
esp_err_t pcf8574_read_bit(pcf8574_handle_t pcf8574_h, uint8_t bit, uint8_t *value);

// пишем биты напрямую
esp_err_t pcf8574_write(pcf8574_handle_t pcf8574_h, uint8_t value);
esp_err_t pcf8574_write_bit(pcf8574_handle_t pcf8574_h, uint8_t bit, uint8_t value);

esp_err_t pcf8574_toggle(pcf8574_handle_t pcf8574_h, uint8_t mask);
esp_err_t pcf8574_toggle_bit(pcf8574_handle_t pcf8574_h, uint8_t bit);

esp_err_t pcf8574_shift_right(pcf8574_handle_t pcf8574_h, const uint8_t n);
esp_err_t pcf8574_shift_left(pcf8574_handle_t pcf8574_h, const uint8_t n);

esp_err_t pcf8574_rotate_right(pcf8574_handle_t pcf8574_h, const uint8_t n);
esp_err_t pcf8574_rotate_left(pcf8574_handle_t pcf8574_h, const uint8_t n);

// регистрация callback для софт-прерывания (polling pcf8574 с интервалом refresh_time_ms)
void pcf8574_register_soft_isr_handler(pcf8574_handle_t pcf8574_h, uint32_t refresh_time_ms, pcf8574_isr_cb cb);

void pcf8574_register_isr_handler(pcf8574_handle_t pcf8574_h, uint8_t gpio_num_isr, pcf8574_isr_cb cb);

void pcf8574_test_task(pcf8574_handle_t pcf8574_h);
#endif //CONFIG_COMPONENT_PCF8574
#endif