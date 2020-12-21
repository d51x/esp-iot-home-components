#pragma once

#ifndef __MQTT_CL_HTTP_H__
#define __MQTT_CL_HTTP_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "http_handlers.h"
#include "httpd.h"
#include "mqtt_cl.h"

extern const char *html_page_config_mqtt;
void mqtt_register_http_print_data();
void mqtt_http_process_params(httpd_req_t *req, void *args);
void mqtt_http_init(httpd_handle_t _server);

#endif