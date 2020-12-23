
#pragma once

#ifndef __PZEM004T_HTTP_H__
#define __PZEM004T_HTTP_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "relay.h"
#include "http_handlers.h"
#include "httpd.h"

#include "pzem004t.h"

#ifdef CONFIG_SENSOR_PZEM004_T_WEB
 
    void pzem_register_http_print_data();  
    void pzem_http_init(httpd_handle_t _server);

#endif

#endif
