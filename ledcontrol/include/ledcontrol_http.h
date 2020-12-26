#pragma once

#ifndef _LEDCONTROL_HTTP_H_
#define _LEDCONTROL_HTTP_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ledcontrol.h"
#include "http_handlers.h"
#include "httpd.h"

#ifdef CONFIG_LED_CONTROL_HTTP

#define LED_CONTROL_URI "/ledc"
#define LED_CONTROL_HANDLERS_COUNT 1

typedef struct ledcontrol_group {
    char *title;
    uint8_t group;
    ledcontrol_handle_t dev_h;
    uint8_t priority;
} ledcontrol_group_t;

extern const char *html_block_led_control_start;
extern const char *html_block_led_control_end;
extern const char *html_block_led_control_data_start;
extern const char *html_block_led_control_item;

void ledcontrol_register_http_print_data(ledcontrol_handle_t dev_h);
void ledcontrol_register_http_handler(httpd_handle_t _server, ledcontrol_handle_t dev_h);
void ledcontrol_http_init(httpd_handle_t _server, ledcontrol_handle_t dev_h);

// добавить новую группу (num, title, priority) или изменить существующую по num
// num - номер группы
void ledcontrol_http_add_group(ledcontrol_handle_t dev_h, const char *title, uint8_t num, uint8_t priority);

#endif
#endif