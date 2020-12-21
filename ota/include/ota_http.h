#pragma once

#ifndef __OTA_HTTP_H__
#define __OTA_HTTP_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "http_handlers.h"
#include "httpd.h"
#include "ota.h"

extern const char *html_page_ota;
void ota_register_http_print_data();
void ota_http_process_params(httpd_req_t *req, void *args);
void ota_http_init(httpd_handle_t _server);

#endif