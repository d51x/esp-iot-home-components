#pragma once

#ifndef _IRRCV_HTTP_H_
#define _IRRCV_HTTP_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "irrcv.h"
#include "http_handlers.h"
#include "http_page_tpl.h"
#include "httpd.h"

#ifdef CONFIG_IR_RECV_HTTP

#define IR_RECV_HANDLERS_COUNT 1
extern const char *IRRCV_URI;


void irrcv_http_init(httpd_handle_t _server, irrcv_handle_t irrcv);

#endif
#endif