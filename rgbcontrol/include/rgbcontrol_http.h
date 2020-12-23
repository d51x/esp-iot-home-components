#pragma once

#ifndef _RGBCONTROL_HTTP_H_
#define _RGBCONTROL_HTTP_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rgbcontrol.h"
#include "ledcontrol_http.h"
#include "http_handlers.h"
#include "httpd.h"

#ifdef CONFIG_RGB_CONTROLLER_HTTP

#define RGB_CONTROL_URI "/colors"
#define RGB_CONTROL_HANDLERS_COUNT 1

extern const char *html_block_rgb_control;    
void rgbcontrol_register_http_print_data(rgbcontrol_handle_t dev_h);
void rgbcontrol_register_http_handler(httpd_handle_t _server, rgbcontrol_handle_t dev_h);
void rgbcontrol_http_init(httpd_handle_t _server, rgbcontrol_handle_t dev_h);

/*
    ip/colors?type=rgb&r=<r>&g=<g>&b=<b>
    ip/colors?type=hsv&h=<h>&s=<s>&v=<v>
    ip/colors?type=int&val=<int_value>
    ip/colors?type=hex&val=<hex_value>
    ip/colors?rgb=<r>,<g>,<b>
    ip/colors?hsv=<h>,<s>,<v>
*/

#endif
#endif