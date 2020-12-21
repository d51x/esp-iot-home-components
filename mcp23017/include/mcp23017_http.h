
#pragma once

#ifndef _MCP23017_HTTP_H_
#define _MCP23017_HTTP_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mcp23017.h"
#include "http_handlers.h"
#include "httpd.h"

#ifdef CONFIG_MCP23017_HTTP

#define MCP23017_URI "/mcp23017"
#define MCP23017_HANDLERS_COUNT 1

extern const char *html_block_mcp23107;    
void mcp23017_print_button( mcp23017_handle_t dev_h, httpd_req_t *req, const char *btn_id, uint8_t idx);
void mcp23017_register_http_print_data(mcp23017_handle_t dev_h);
void mcp23017_register_http_handler(httpd_handle_t _server, mcp23017_handle_t dev_h);
void mcp23017_http_init(httpd_handle_t _server, mcp23017_handle_t dev_h);
void mcp23017_http_set_btn_name(mcp23017_handle_t dev_h, uint8_t idx, const char *name); 
#endif
#endif