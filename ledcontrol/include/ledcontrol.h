#pragma once

#ifndef __LEDCONTROL_H__
#define __LEDCONTROL_H__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/pwm.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "utils.h"
#include "http_utils.h"

#ifdef CONFIG_LED_CONTROLLER

#define LEDCONTROL_CHANNEL_MAX 5
#define LEDCONTROL_FREQ_MIN 100
#define LEDCONTROL_FREQ_MAX 500
#define MAX_DUTY 255

typedef enum {
	TBL_NONE = 0,
	TBL_LOG,
	TBL_PWR,
	TBL_AVG,
	TBL_32B,
	TBL_MAX
} brightness_table_e;

#define TBL_LOG_BRIGHTNESS_STEPS 19
#define TBL_POW_BRIGHTNESS_STEPS 19
#define TBL_AVG_BRIGHTNESS_STEPS 19
#define TBL_32B_BRIGHTNESS_STEPS 31

//================================================================================================================================
static uint8_t bright_steps[TBL_MAX] = { MAX_DUTY, TBL_LOG_BRIGHTNESS_STEPS, TBL_POW_BRIGHTNESS_STEPS, TBL_AVG_BRIGHTNESS_STEPS, TBL_32B_BRIGHTNESS_STEPS};

static uint32_t tbl_log_brightness[TBL_LOG_BRIGHTNESS_STEPS+1] = {0,1,2,3,4,5,7,9,12,16,21,28,37,48,64,84,111,147,194,255};  
									// 0-100%, логарифмическая, S1 = k * ln(R), k=18.04, R - значение ШИМ (0-255)

static uint32_t tbl_pow_brightness[TBL_POW_BRIGHTNESS_STEPS+1] = {0,1,2,3,4,7,11,16,23,31,42,54,69,87,107,130,156,186,219,255};  
									// 0-100%, степенная, S2 = k * R^n, k=16.06, n=0.33,  R - значение ШИМ (0-255)
									
static uint32_t tbl_avg_brightness[TBL_AVG_BRIGHTNESS_STEPS+1] = {0,1,2,3,4,6,8,12,16,22,29,39,51,67,86,109,136,170,209,255};  
									// 0-100%, средняя, S=(S1+S2)/2
	
//32 bit
static uint32_t tbl_32b_brightness[TBL_32B_BRIGHTNESS_STEPS+1] = {0,1,2,3,5,8,12,16,21,26,32,38,45,52,60,68,76,85,95,105,115,125,136,148,160,172,185,198,211,225,239,255};

static uint32_t *tbl_brightness[TBL_MAX] = {NULL, tbl_log_brightness, tbl_pow_brightness, tbl_avg_brightness, tbl_32b_brightness};
//================================================================================================================================

typedef enum {
    UP,
    DOWN
} direction_e;

typedef void *ledcontrol_handle_t;      // ledcontrol object
typedef void *ledcontrol_channel_handle_t;      // ledcontrol_channel object

typedef struct ledcontrol ledcontrol_t;
typedef struct ledcontrol_channel ledcontrol_channel_t;

typedef void (* ledcontrol_init_f)();  // инициализация ledcontrol и pwm
typedef esp_err_t (* ledcontrol_register_channel_f)(ledcontrol_channel_t );  // регистрация канала
typedef esp_err_t (* ledcontrol_set_duty_f)(ledcontrol_channel_t *, uint16_t);  // установить duty канала
typedef uint16_t (* ledcontrol_get_duty_f)(ledcontrol_channel_t *);  // получить duty канала
typedef void (* ledcontrol_update_f)();  // aka pwm start
typedef esp_err_t (* ledcontrol_channel_on_f)(ledcontrol_channel_t *);  
typedef esp_err_t (* ledcontrol_channel_off_f)(ledcontrol_channel_t *);  
typedef esp_err_t (* ledcontrol_channel_next_duty_f)(ledcontrol_channel_t *, uint8_t);  
typedef esp_err_t (* ledcontrol_channel_prev_duty_f)(ledcontrol_channel_t *, uint8_t);  
typedef esp_err_t (* ledcontrol_channel_fade_f)(ledcontrol_channel_t *, uint16_t, uint16_t, uint16_t);  
typedef esp_err_t (* ledcontrol_channel_fade_to_off_f)(ledcontrol_channel_t *, uint16_t, uint16_t);  
typedef esp_err_t (* ledcontrol_channel_fade_to_on_f)(ledcontrol_channel_t *, uint16_t, uint16_t);  
typedef void (* ledcontrol_channel_set_brightness_table_f)(ledcontrol_channel_t *, brightness_table_e);  
typedef void (* ledcontrol_all_off_f)();  
typedef void (* ledcontrol_all_on_f)();  
typedef void (* ledcontrol_all_next_duty_f)(uint8_t);
typedef void (* ledcontrol_all_prev_duty_f)(uint8_t);
typedef void (* ledcontrol_all_fade_f)(uint16_t, uint16_t, uint16_t);
typedef void (* ledcontrol_all_fade_to_off_f)(uint16_t, uint16_t);
typedef void (* ledcontrol_all_fade_to_on_f)(uint16_t, uint16_t);		
		
struct ledcontrol_channel {
		uint8_t pin;
		uint8_t channel;
		uint16_t duty;
		bool inverted;
		brightness_table_e bright_tbl;
		ledcontrol_t *ledc;
		char *name;
		uint8_t group;
};

struct ledcontrol {
	uint16_t freq;				// частота pwm
	uint8_t led_cnt;			// кол-во каналов
	ledcontrol_channel_t *channels; // указатель на массив каналов, нужен для инициализации pwm
	
	// указатели на функции
	ledcontrol_init_f init;
	ledcontrol_register_channel_f register_channel;
	ledcontrol_set_duty_f set_duty;
	ledcontrol_get_duty_f get_duty;
	ledcontrol_update_f update;
	ledcontrol_channel_on_f on;
	ledcontrol_channel_off_f off;
	ledcontrol_channel_next_duty_f next_duty;
	ledcontrol_channel_prev_duty_f prev_duty;
	ledcontrol_channel_fade_f fade;
	ledcontrol_channel_fade_to_off_f fade_to_off;
	ledcontrol_channel_fade_to_on_f fade_to_on;
	ledcontrol_channel_set_brightness_table_f bright_tbl;
	ledcontrol_all_off_f off_all;
	ledcontrol_all_on_f on_all;
	ledcontrol_all_next_duty_f next_duty_all;
	ledcontrol_all_prev_duty_f prev_duty_all;
	ledcontrol_all_fade_f fade_all;
	ledcontrol_all_fade_to_off_f fade_to_off_all;
	ledcontrol_all_fade_to_on_f fade_to_on_all;
	
};


// здесь укажем только внешние функции
// создать объект ledcontrol, потом надо создать каналы, потом зарегистрировать каналы, потом ledcontrol_init для инициализации pwm
ledcontrol_handle_t* ledcontrol_create(uint32_t freq_hz, uint8_t channel_cnt);

// создать канал, сначала надо создать каналы
//esp_err_t ledcontrol_register_channel(ledcontrol_channel_t ledc_ch);

/*

At first, create led_controller object
    ledcontrol_t *ledc;
    ledcontrol_t* ledc_h = ledcontrol_create(LED_FREQ_HZ, LED_CTRL_CNT);
    ledc = (ledcontrol_t *)ledc_h;

then register rgb-channels
    ledc->register_channel(ch_red);
    ledc->register_channel(ch_green);
    ledc->register_channel(ch_blue);

    where ledcontrol_channel_t ch_red, ch_green, ch_blue (with pin and channel)

Initialize led_Controller object
	ledc->init();

now you can change brightness of one channel or all at the same time
    ledc->fade( hsv )
    

to control via http get request you need add a get request handler
    add_uri_get_handler( http_server, ledc->uri, ledc->http_get_handler);

    ip/ledc?ch=<channel>&duty=<duty>
    ip/ledc?ch=<channel>&on=1
    ip/ledc?ch=<channel>&off=1
    ip/ledc?ch=<channel>&step=<step>   // next duty
    ip/ledc?ch=<channel>&fade=1&from=<duty_from>&to=<duty_to>&delay=<duty_delay>
    ip/ledc?allon=1
    ip/ledc?alloff=1
*/
#endif
#endif