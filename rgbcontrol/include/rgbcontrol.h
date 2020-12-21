
#ifndef __RGBCONTROL_H__
#define __RGBCONTROL_H__


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
#include "freertos/task.h"
#include "utils.h"
#include "ledcontrol.h"
#include "colors.h"
#include "effects.h"
#include "http_utils.h"

#ifdef CONFIG_RGB_CONTROLLER

#define RGB_DEFAULT_FADE 1000
#define RGB_DEFAULT_FADEUP 40
#define RGB_DEFAULT_FADEDOWN 40

#ifdef CONFIG_RGB_EFFECTS
#define MQTT_TOPIC_EFFECT_NAME "effect/name"
#define MQTT_TOPIC_EFFECT_ID "effect/id"
#endif

#define MQTT_TOPIC_COLOR_INT "color/int"

typedef void *rgbcontrol_handle_t;      // rgbcontrol object
typedef struct rgbcontrol rgbcontrol_t;

typedef void (* rgbcontrol_set_color_hsv_f)(color_hsv_t hsv);  
typedef void (* rgbcontrol_set_color_rgb_f)(color_rgb_t rgb);  
typedef void (* rgbcontrol_set_color_int_f)(uint32_t color32);  
typedef void (* rgbcontrol_set_color_hex_f)(const char *hex);  

typedef void (* rgbcontrol_set_brightness_f)(int8_t value);
typedef void (* rgbcontrol_fade_brightness_f)(int8_t brightness_from, int8_t brightness_to, int16_t brightness_delay);
typedef void (* rgbcontrol_inc_brightness_f)(int8_t step);
typedef void (* rgbcontrol_dec_brightness_f)(int8_t step);

typedef void (* rgbcontrol_set_saturation_f)(int8_t value);
typedef void (* rgbcontrol_fade_saturation_f)(int8_t saturation_from, int8_t saturation_to, int16_t saturation_delay);
typedef void (* rgbcontrol_inc_saturation_f)(int8_t step);
typedef void (* rgbcontrol_dec_saturation_f)(int8_t step);

#ifdef CONFIG_RGB_EFFECTS
typedef void (* rgbcontrol_set_effects_f)(void *effects);
#endif

struct rgbcontrol {
	color_hsv_t hsv;
    ledcontrol_t *ledc;
	ledcontrol_channel_t red; 
	ledcontrol_channel_t green; 
	ledcontrol_channel_t blue; 
	int fade_delay;
    int fadeup_delay;
    int fadedown_delay;



    
	// указатели на функции
	rgbcontrol_set_color_hsv_f      set_color_hsv;
	rgbcontrol_set_color_rgb_f      set_color_rgb;
	rgbcontrol_set_color_int_f      set_color_int;
	rgbcontrol_set_color_hex_f      set_color_hex;

    rgbcontrol_set_brightness_f     set_brightness;
    rgbcontrol_fade_brightness_f    fade_brightness;
    rgbcontrol_inc_brightness_f     inc_brightness;
    rgbcontrol_dec_brightness_f     dec_brightness;

    rgbcontrol_set_saturation_f     set_saturation;
    rgbcontrol_fade_saturation_f    fade_saturation;
    rgbcontrol_inc_saturation_f     inc_saturation;
    rgbcontrol_dec_saturation_f     dec_saturation;

    #ifdef CONFIG_RGB_EFFECTS
    int effect_id; // TODO: переделать на указатель 
    void *effects;
    rgbcontrol_set_effects_f     set_effects;    
    #endif    


};

typedef enum {
    RGB_COLOR_INT,
    RGB_EFFECT_ID,
    RGB_EFFECT_NAME
} rgbcontrol_queue_type_t;

typedef struct rgbcontrol_queue {
    uint32_t data;
    rgbcontrol_queue_type_t type;
} rgbcontrol_queue_t;

QueueHandle_t rgbcontrol_color_queue;

// здесь укажем только внешние функции
// создать объект rgbcontrol
rgbcontrol_t* rgbcontrol_init(ledcontrol_t *ledc, ledcontrol_channel_t *red, ledcontrol_channel_t *green, ledcontrol_channel_t *blue);



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

then initialize rgb_controller 
    rgbcontrol_t *rgb_ledc;
    rgb_ledc = rgbcontrol_init(ledc, ch_red, ch_green, ch_blue);

now you can change color and make animations with led
    rgb_ledc->set_color_hsv( hsv )
    rgb_ledc->set_color_rgb( rgb )
    rgb_ledc->set_color_hex( hex )
    rgb_ledc->set_color_int( int )
    rgb_ledc->set_brightness( int )
    rgb_ledc->inc_brightness( int )
    rgb_ledc->dec_brightness( int )
    rgb_ledc->fade_brightness( from to with delay )
    rgb_ledc->set_saturation( int )
    rgb_ledc->inc_saturation( int )
    rgb_ledc->dec_saturation( int )
    rgb_ledc->fade_saturation( from to with delay )

    ip/colors?type=rgb&r=<r>&g=<g>&b=<b>
    ip/colors?type=hsv&h=<h>&s=<s>&v=<v>
    ip/colors?type=int&val=<int_value>
    ip/colors?type=hex&val=<hex_value>
    ip/colors?rgb=<r>,<g>,<b>
    ip/colors?hsv=<h>,<s>,<v>
*/
#endif

#endif