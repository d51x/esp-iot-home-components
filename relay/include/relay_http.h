
#pragma once

#ifndef _RELAY_HTTP_H_
#define _RELAY_HTTP_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "relay.h"
#include "http_handlers.h"
#include "httpd.h"

#ifdef CONFIG_RELAY_HTTP

#define RELAY_URI "/relay"
#define RELAY_HANDLERS_COUNT 1

extern const char *html_block_relays;  

void relay_print_button(httpd_req_t *req, const char *btn_id, uint8_t idx);
void relay_http_init(httpd_handle_t _server);

#endif
#endif