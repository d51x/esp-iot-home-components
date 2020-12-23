
#pragma once

#ifndef __HTTP_PAGE_H__
#define __HTTP_PAGE_H__



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_http_server.h"
#include "esp_err.h"
#include "rom/ets_sys.h"


#include "utils.h"
#include "wifi.h"
#include "mqtt_cl.h"
#include "http_utils.h"
#include "http_handlers.h"



#define STR_ON "ВКЛ"
#define STR_OFF "ВЫКЛ"
#define STR_YES "ДА"
#define STR_NO "НЕТ"

typedef struct http_args{
    void *dev;
    httpd_req_t *req;
} http_args_t;

typedef void (* func_http_print_page_block)(http_args_t *args);  
typedef esp_err_t (*httpd_uri_process_fn)(httpd_req_t *req, void *args);

extern uint8_t http_print_page_block_count;



typedef struct {
    const char uri[12]; 
    uint32_t index;
    func_http_print_page_block fn_print_block;
    void *args1;
    char name[20];
    httpd_uri_process_fn process_cb;
    void *args2;
} http_print_page_block_t;

extern http_print_page_block_t *http_print_page_block;


void show_custom_page(httpd_req_t *req, const char *uri, const char *title);

void show_page_main(httpd_req_t *req, const char *title);
void show_page_setup(httpd_req_t *req,  const char *title);
void show_page_config(httpd_req_t *req, const char *title);
void show_page_tools(httpd_req_t *req, const char *title);
void show_page_ota(httpd_req_t *req, const char *title);
void show_page_debug(httpd_req_t *req, const char *title);


void page_generate_html_start(httpd_req_t *req, const char *title);
void page_generate_html_end(httpd_req_t *req);
void page_generate_top_header(httpd_req_t *req);
void page_show_menu(httpd_req_t *req);
void page_initialize_menu();

//void page_generate_data(const char *uri, char *data);
void page_generate_data(httpd_req_t *req, const char *uri);





void show_http_page(httpd_req_t *req);



void show_restart_page_data(httpd_req_t *req);
void show_restarting_page_data(httpd_req_t *req);



void httpd_resp_sendstr_chunk(httpd_req_t *req, const char *buf);
void httpd_resp_end(httpd_req_t *req);



void set_redirect_header(uint8_t time, const char *uri, char *data);

// зарегистрировать функцию колбека, которая будет вызвана при выводе информации на страницу
// uri - на какой странице выводить
// index - очередность вывода
// fn_cb - функция коллбека для формирования буфера
esp_err_t register_print_page_block(const char *name, const char *uri, uint8_t index, func_http_print_page_block fn_cb, http_args_t *args1, httpd_uri_func fn_cb2, void *args2);


esp_err_t register_http_page_menu(const char *uri, const char *name);

void http_print_value(httpd_req_t *req, const char *html_label, const char *title, const char *fmt, type_e type, void *value);
void http_print_button(httpd_req_t *req, const char *b_id, const char *class, const char *st_class, const char *uri, int value, const char *text, int st, int v);

void httpd_resp_sendstr_chunk_fmt(httpd_req_t *req, const char *fmt, ...);

#endif 