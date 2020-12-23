#pragma once

#ifndef _I2C_HTTP_H_
#define _I2C_HTTP_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "http_handlers.h"
#include "http_page_tpl.h"
#include "httpd.h"
#include "i2c_bus.h"

#ifdef CONFIG_COMPONENT_I2C

#ifdef CONFIG_COMPONENT_I2C_SCANNER
    #define I2C_HANDLERS_COUNT 1
#endif
void i2c_register_http_print_data();
void i2c_register_http_handler(httpd_handle_t _server);
void i2c_http_process_params(httpd_req_t *req, void *args);
void i2c_http_init(httpd_handle_t _server);

#ifdef CONFIG_COMPONENT_I2C_SCANNER
esp_err_t i2cscan_get_handler(httpd_req_t *req);
#endif

#endif //#ifdef CONFIG_COMPONENT_I2C

#endif