#pragma once

#ifndef __HTTPD_H__
#define __HTTPD_H__


#include "esp_log.h"
#include "esp_system.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include <sys/param.h>


//#include "http_utils.h"
//#include "core.h"
//#include "http_page.h"

#include "nvsparam.h"
//#include "mqtt.h"





#define WEB_SERVER_STACK_SIZE       CONFIG_HTTP_SERVER_STACK_SIZE //1024*8*2

esp_err_t _http_event_handler(esp_http_client_event_t *evt);
typedef esp_err_t (*httpd_uri_func)(httpd_req_t *req);

extern uint8_t http_handlers_count;
//int uri_handlers_no = 20;
//httpd_uri_t uri_handlers[];


void webserver_init(httpd_handle_t* _server);
httpd_handle_t webserver_start(void);
void webserver_stop(httpd_handle_t server);
void register_uri_handlers(httpd_handle_t _server);
void add_uri_get_handler(httpd_handle_t _server, const char *uri, httpd_uri_func func, void *ctx);
void add_uri_post_handler(httpd_handle_t _server, const char *uri, httpd_uri_func func, void *ctx);
void make_redirect(httpd_req_t *req, uint8_t timeout, const char *path);

#endif /* __HTTPD_H__ */