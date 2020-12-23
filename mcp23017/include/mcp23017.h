#pragma once

#ifndef _MCP23017_H_
#define _MCP23017_H_

#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "nvsparam.h"
#include "i2c_bus.h"
#include "freertos/task.h"

#include "utils.h"

#ifdef CONFIG_COMPONENT_MCP23017

#define GPIO_EXT_START 200
#define MCP23017_I2C_ADDRESS_DEFAULT   (0x20)           //0100A2A1A0+R/W  // 7-bit i2c addresses MUST be shifted left 1 bit (chibios does this for us)
#define MCP23017_INTA_GPIO_DEFAULT      CONFIG_MCP23017_ISR_INTA_GPIO // 4          
#define MCP23017_INTB_GPIO_DEFAULT      CONFIG_MCP23017_ISR_INTB_GPIO // 5
#define MCP23017_GPIO_INPUTS_DEFAULT    CONFIG_MCP23017_GPIO_INPUTS // 0xFF00

#define MCP23017_PORT_A_BYTE(x)         (x & 0xFF)                      //get pin of GPIOA
#define MCP23017_PORT_B_BYTE(x)         (x >> 8)                        //get pin of GPIOB
#define MCP23017_PORT_AB_WORD(buff)     (buff[0] | (buff[1] << 8))      //get pin of GPIOA and pin of GPIOB

typedef void (*interrupt_cb)(void *arg);

typedef void* mcp23017_handle_t;                        /*handle of mcp23017*/
typedef uint8_t mcp23017_gpio_t;

typedef enum mcp23017_status {
	MCP23017_DISABLED,
	MCP23017_ENABLED
} mcp23017_status_t;

typedef struct mcp23017_pin_isr {
        uint8_t pin;
        interrupt_cb pin_cb;
        gpio_int_type_t intr_type;
        void *args;
} mcp23017_pin_isr_t;

typedef struct mcp23017 {
	uint8_t addr;
        uint16_t pins_values;                           // значения пинов
        uint16_t pins_saved;                            // пины, значения которых надо сохранять
        uint16_t pins_direction;                        // типы пина - вход или выход
        uint16_t pins_invert;                           // инвертированные пины
	uint16_t pins_interrupt;                        // пины с прерываниями
	uint16_t pins_def_val;                          // дефолтные значения ног, прерывание сработает, если на ноге сигнал отличается от дефолтного
        uint16_t pins_condition;                        // условия сработки прерывания на ногах
	mcp23017_status_t status;
        i2c_bus_handle_t i2c_bus_handle;
        // interrupts
        #ifdef CONFIG_MCP23017_ISR
        uint8_t int_a_pin; // esp pin for INTA
        uint8_t int_b_pin; // esp pin for INTB
        TaskHandle_t task;
        interrupt_cb isr_cb;
        QueueHandle_t taskq;
	QueueHandle_t argq;
        mcp23017_pin_isr_t *pin_isr; // указатель на массив коллбеков для пинов
        uint8_t pin_isr_cnt;
        #ifdef CONFIG_MCP23017_HTTP
        uint16_t http_buttons;
        char *names[16];
        #endif
        #endif
} mcp23017_t;

QueueHandle_t mcp23017_status_queue;

mcp23017_handle_t mcp23017_create(uint8_t addr);
esp_err_t mcp23017_delete(mcp23017_handle_t dev_h);
esp_err_t mcp23017_enable(mcp23017_handle_t dev_h);
esp_err_t mcp23017_disable(mcp23017_handle_t dev_h);

esp_err_t mcp23017_write_reg(mcp23017_handle_t dev_h, uint8_t reg_addr, uint8_t value);
esp_err_t mcp23017_write_pin(mcp23017_handle_t dev_h, uint8_t pin, uint8_t val);
esp_err_t mcp23017_write(mcp23017_handle_t dev_h, uint8_t reg_start, uint8_t reg_cnt, uint8_t *data);

esp_err_t mcp23017_read(mcp23017_handle_t dev_h, uint8_t reg_start, uint8_t reg_cnt, uint8_t *data );
esp_err_t mcp23017_read_reg(mcp23017_handle_t dev_h, uint8_t reg_addr, uint8_t *value);
esp_err_t mcp23017_read_pin(mcp23017_handle_t dev_h, uint8_t pin, uint8_t *val);

esp_err_t mcp23017_set_interrupts(mcp23017_handle_t dev_h, uint16_t pins);
esp_err_t mcp23017_set_defaults(mcp23017_handle_t dev_h, uint16_t defaultValues);
esp_err_t mcp23017_set_conditions(mcp23017_handle_t dev_h, uint16_t conditions);

esp_err_t mcp23017_set_directions(mcp23017_handle_t dev_h, uint16_t directions);
esp_err_t mcp23017_set_inversions(mcp23017_handle_t dev_h, uint16_t pins);

esp_err_t mcp23017_write_io(mcp23017_handle_t dev_h, uint16_t value);
esp_err_t mcp23017_read_io(mcp23017_handle_t dev_h, uint16_t *data);

esp_err_t mcp23017_read_pin(mcp23017_handle_t dev_h, uint8_t pin, uint8_t *val);
esp_err_t mcp23017_write_pin(mcp23017_handle_t dev_h, uint8_t pin, uint8_t val);

#ifdef CONFIG_MCP23017_ISR
esp_err_t mcp23017_isr_handler_add(mcp23017_handle_t dev_h, uint8_t pin, gpio_int_type_t intr_type, interrupt_cb cb, void *args);
#endif

#endif // COMPONENT
#endif

