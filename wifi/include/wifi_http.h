#pragma once

#ifndef __WIFI_HTTP_H__
#define __WIFI_HTTP_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "http_handlers.h"
#include "httpd.h"
#include "wifi.h"

extern const char *html_page_config_wifi;
void wifi_register_http_print_data();
void wifi_http_process_params(httpd_req_t *req, void *args);
void wifi_http_init(httpd_handle_t _server);

#endif