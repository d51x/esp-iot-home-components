

#pragma once

#ifndef _SENSORS_HTTP_H_
#define _SENSORS_HTTP_H_



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "http_handlers.h"
#include "httpd.h"
#include "sensors.h"


#define SENSORS_URI "/sensors"

void sensors_http_init(httpd_handle_t _server);


#endif