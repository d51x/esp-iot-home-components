

#pragma once

#ifndef _SHT21_HTTP_H_
#define _SHT21_HTTP_H_



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "http_handlers.h"
#include "httpd.h"
#include "sht21.h"

#ifdef CONFIG_SENSOR_SHT21


extern const char *html_block_sht21;    
void sht21_register_http_print_data();  
void sht21_http_init(httpd_handle_t _server);

#endif //CONFIG_SENSOR_SHT21

#endif