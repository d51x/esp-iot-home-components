#pragma once

#ifndef __EFFECTS_H__
#define __EFFECTS_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "colors.h"
#include "rgbcontrol.h"


#ifdef CONFIG_RGB_EFFECTS

#define COLOR_EFFECTS_MAX 13
#define EFFECT_STOP COLOR_EFFECTS_MAX-1

#define EFFECT_FADE_NO_DELAY 0

#define EFFECT_FADE_DELAY 1000
#define EFFECT_FADEUP_DELAY 40
#define EFFECT_FADEDOWN_DELAY 60

#define EFFECT_NAME_LEN 10

typedef void (* effect_f)();
typedef void (* effectcb_f)(void *arg);
typedef void (* effect_set_color_hsv_f)(color_hsv_t hsv);  
typedef void (* effect_set_effect_by_name_f)(const char  *name );  
typedef void (* effect_set_effect_f)(int8_t id);  
  

typedef enum {
    STOP,
    JUMP,               // only on 
    FADE,               // fade up & fade down
    RANDOM_JUMP,        
    RANDOM_FADE,
    RANDOM_CB              // all random - color, brightness,
} color_effect_e;

typedef enum {
    E_UP,
    E_DOWN
} effect_direction_e;

typedef struct effect effect_t;
typedef struct effects effects_t;

struct effect {
    char name[EFFECT_NAME_LEN];
    color_effect_e type;
    int16_t fadeup_delay;
    int16_t fadedown_delay;
    effectcb_f cb;
    uint16_t *colors;
    uint16_t colors_cnt;
    color_hsv_t hsv;
    effects_t *pe;          // pinter to parent object 
    uint16_t mm;
};

struct effects {
    effect_t *effect;
    void  *rgbctrl;
    int8_t effect_id;
    effect_set_color_hsv_f set_color_hsv; // указатель на функцию set_color_hsv родительского объекта rgb_control

    effect_set_effect_by_name_f set_by_name;
    effect_set_effect_f set;
    effect_f next;
    effect_f prev;
    effect_f stop;      // stop current effect
    TaskHandle_t task;  // task with effect
    effect_f task_cb;   // effect callback function
};

void effect_jump3(void *arg);
void effect_jump7(void *arg);
void effect_jump12(void *arg);
void effect_rndjump7(void *arg);
void effect_rndjump12(void *arg);
void effect_fade3(void *arg);
void effect_fade7(void *arg);
void effect_fade12(void *arg);
void effect_rndfade7(void *arg);
void effect_rndfade12(void *arg);
void effect_wheel(void *arg);
void effect_rnd(void *arg);
void effect_stop(void *arg);

// EFFECT_NAME_LEN = 10
static effect_t color_effects[COLOR_EFFECTS_MAX]  = {
    { "jump3",      JUMP,           EFFECT_FADE_DELAY,      EFFECT_FADE_NO_DELAY,   effect_jump3 },     // 0
    { "jump7",      JUMP,           EFFECT_FADE_DELAY,      EFFECT_FADE_NO_DELAY,   effect_jump7 },     // 1   
    { "jump12",     JUMP,           EFFECT_FADE_DELAY,      EFFECT_FADE_NO_DELAY,   effect_jump12 },     // 2 
    { "rndjump7",   RANDOM_JUMP,    EFFECT_FADE_DELAY,      EFFECT_FADE_NO_DELAY,   effect_rndjump7 },     // 3
    { "rndjump12",  RANDOM_JUMP,    EFFECT_FADE_DELAY,      EFFECT_FADE_NO_DELAY,   effect_rndjump12 },     // 4 
    { "fade3",      FADE,           EFFECT_FADEUP_DELAY,    EFFECT_FADEDOWN_DELAY,  effect_fade3},    // 5
    { "fade7",      FADE,           EFFECT_FADEUP_DELAY,    EFFECT_FADEDOWN_DELAY,  effect_fade7},    // 6
    { "fade12",     FADE,           EFFECT_FADEUP_DELAY,    EFFECT_FADEDOWN_DELAY,  effect_fade12},    // 7 
    { "rndfade7",   RANDOM_FADE,    EFFECT_FADEUP_DELAY,    EFFECT_FADEDOWN_DELAY,  effect_rndfade7},    // 8 
    { "rndfade12",  RANDOM_FADE,    EFFECT_FADEUP_DELAY,    EFFECT_FADEDOWN_DELAY,  effect_rndfade12},    // 9 
    { "wheel",      JUMP,           EFFECT_FADE_DELAY,      EFFECT_FADE_NO_DELAY,   effect_wheel},     // 10
    { "rnd",        RANDOM_CB,      EFFECT_FADE_DELAY,      EFFECT_FADE_NO_DELAY,   effect_rnd},     // 11
    { "stop",       STOP,           EFFECT_FADE_DELAY,      EFFECT_FADE_NO_DELAY,   effect_stop},     // 12
                                
};

effects_t* effects_init(void *rgbctrl, effect_set_color_hsv_f *cb);
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

initialize effects and set effects to rgb_controller
    effects_t* effects;
    effects = effects_init( rgb_ledc, rgb_ledc->set_color_hsv );
    rgb_ledc->set_effects( effects );

now you can change effects by id or by name
    effects->set(id)
    effects->set_by_name( name )
    effects->next()
    effects->prev()
    effects->stop()

to control via http get request you need add a get request handler
    add_uri_get_handler( http_server, rgb_ledc->uri, rgb_ledc->http_get_handler);

    ip/colors?type=effect&id=<id>
    ip/colors?type=effect&name=<name>
*/


#endif
#endif