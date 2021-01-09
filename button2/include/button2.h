#ifndef __BUTTON2_H__
#define __BUTTON2_H__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/timers.h"

typedef void (* button_cb)(void *); // указатель на функцию call back ???


typedef enum {
    BUTTON_ACTIVE_HIGH = 1,    /*!<button active level: high level*/
    BUTTON_ACTIVE_LOW = 0,     /*!<button active level: low level*/
} button_active_t;


typedef struct {
    button_cb cb;
    void *args;
    uint16_t timeout;   // задержка для long_press
    uint8_t idx;  //short press index
} btn_cb_t;

typedef struct {
	uint8_t pin;
	uint8_t level;
	//button_cb *short_cb;
	//void *short_args;
    btn_cb_t *short_press_cb;
    uint8_t short_press_cb_cnt;

    btn_cb_t *long_press_cb;
    uint8_t long_press_cb_cnt;

	//os_timer_t read_timer;
    TimerHandle_t  read_timer;
	//os_timer_t short_pressed_timer;	
	TimerHandle_t  short_press_timer;

	uint8_t short_press_count;
	uint32_t long_press_time;
	uint8_t state;
	
	uint32_t pressed_time;
	uint32_t prev_pressed_time;
	uint32_t ms;
	uint32_t released_time;
	uint32_t hold_time;
	uint32_t wait_time;
} button_t;


button_t *create_button(gpio_num_t gpio_num, button_active_t active_level);

void button_add_short_press(button_t *btn, uint8_t press_cnt, button_cb cb, void *args);
void button_add_long_press(button_t *btn, uint16_t timeout, button_cb cb, void *args);

#endif