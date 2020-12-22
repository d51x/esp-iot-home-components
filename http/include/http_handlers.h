#pragma once

//#ifndef __HTTP_HANDLERS_H__
//#define __HTTP_HANDLERS_H__



#include "esp_http_client.h"
#include "utils.h"
#include "http_utils.h"
#include "http_page.h"

#include "ota.h"
#include "wifi.h"

#include "mqtt_cl.h"


/*
pages:

  menu: 
      main - main page
      setup - auth, wifi type and auth for sta 
*/
#define WEB_SERVER_MAX_URI_HANDLERS CONFIG_HTTP_HANDLERS_COUNT // + PAGE_URI_MAX //40

#define HTTP_STR_MAIN "Main"
#define HTTP_STR_SETUP "Setup"
#define HTTP_STR_CONFIG "Config"
#define HTTP_STR_DEBUG "Debug"
#define HTTP_STR_TOOLS "Tools"
#define HTTP_STR_OTA "Update"
#define HTTP_STR_REBOOT "Reboot"

#define HTTP_URI_ROOT "/"
#define HTTP_URI_SETUP "/setup"
#define HTTP_URI_TOOLS "/tools"
#define HTTP_URI_CONFIG "/config"
#define HTTP_URI_OTA "/update"
#define HTTP_URI_OTAPOST "/update"
#define HTTP_URI_DEBUG "/debug"
#define HTTP_URI_REBOOT "/reboot"
#define HTTP_URI_ICON_MENU "/menu.png"
#define HTTP_URI_ICON_MENU2 "/menu2.png"

#define HTTP_MEDIA_TYPE_ICON "image/x-icon"

typedef enum {
    HTML_PAGE_CFG_WIFI = 1,
    HTML_PAGE_CFG_MQTT,
    HTML_PAGE_CFG_OTA
    
} html_page_cfg_num_t;

typedef void (* func_http_show_page)(void *t, void *d); 

typedef struct user_ctx {
    char title[20];
    uint8_t show;
    func_http_show_page fn;
    void *args;
} user_ctx_t;

enum {
    PAGE_URI_ROOT = 0,
    PAGE_URI_SETUP,
    PAGE_URI_DEBUG,
    PAGE_URI_CONFIG,
    PAGE_URI_TOOLS,
    PAGE_URI_OTA,
    PAGE_URI_REBOOT,
    PAGE_URI_CSS,
    PAGE_URI_AJAX,
    PAGE_URI_FAVICO,
    PAGE_URI_ICON_MENU,
    PAGE_URI_ICON_MENU2,
    PAGE_URI_MAX
} pages_uri_index_e;

extern const char *PAGES_URI[PAGE_URI_MAX];
extern user_ctx_t PAGES_HANDLER[PAGE_URI_MAX];

#define MENU_ITEM_COUNT 6
#define MENU_ITEM_LENGTH 10

extern uint8_t menu_items_count;

typedef struct http_menu_item {
  char uri[MENU_ITEM_LENGTH];
  char name[MENU_ITEM_LENGTH];
} http_menu_item_t;

extern http_menu_item_t *http_menu;

    //extern const unsigned char device_png_start[] asm("_binary_device_png_start");
    //extern const unsigned char device_png_end[]   asm("_binary_device_png_end");

    //extern const unsigned char memory_png_start[] asm("_binary_memory_png_start");
    //extern const unsigned char memory_png_end[]   asm("_binary_memory_png_end");

    extern const unsigned char menu_png_start[] asm("_binary_menu_png_start");
    extern const unsigned char menu_png_end[]   asm("_binary_menu_png_end");

    extern const unsigned char menu2_png_start[] asm("_binary_menu2_png_start");
    extern const unsigned char menu2_png_end[]   asm("_binary_menu2_png_end");


    //extern const unsigned char uptime_png_start[] asm("_binary_uptime_png_start");
    //extern const unsigned char uptime_png_end[]   asm("_binary_uptime_png_end");


   // extern const unsigned char wifi_png_start[] asm("_binary_wifi_png_start");
    //extern const unsigned char wifi_png_end[]   asm("_binary_wifi_png_end");

esp_err_t main_get_handler(httpd_req_t *req);
esp_err_t setup_get_handler(httpd_req_t *req);
esp_err_t config_get_handler(httpd_req_t *req);
esp_err_t tools_get_handler(httpd_req_t *req);
esp_err_t debug_get_handler(httpd_req_t *req);
esp_err_t update_get_handler(httpd_req_t *req);
esp_err_t update_post_handler(httpd_req_t *req);
esp_err_t reboot_get_handler(httpd_req_t *req);
esp_err_t favicon_get_handler(httpd_req_t *req);
esp_err_t icons_get_handler(httpd_req_t *req);



/*
esp_err_t icon_device_get_handler(httpd_req_t *req);
esp_err_t icon_memory_get_handler(httpd_req_t *req);
esp_err_t icon_menu_get_handler(httpd_req_t *req);
esp_err_t icon_menu2_get_handler(httpd_req_t *req);
esp_err_t icon_uptime_get_handler(httpd_req_t *req);
esp_err_t icon_wifi_get_handler(httpd_req_t *req);
*/

esp_err_t main_css_get_handler(httpd_req_t *req);
esp_err_t main_ajax_get_handler(httpd_req_t *req);

// uri - параметры какого uri будем обрабатывать
// fn_cb - функция коллбека обработки данных

esp_err_t register_http_process_page_data(const char *uri, httpd_uri_func fn_cb);

/*
httpd_uri_t uri_handlers[] = {
    { .uri      = "/",
      .method   = HTTP_GET,
      .handler  = main_get_handler,
      .user_ctx = "Main page",          // указатель на функцию отрисовки страницы + тайтл страницы
    },   
    { .uri      = "/setup",
      .method   = HTTP_GET,
      .handler  = setup_get_handler,
      .user_ctx = "/setup",
    },
    {   .uri   = "/debug",
        .method    = HTTP_GET,
        .handler   = debug_get_handler,
        .user_ctx  = "Debug page",
    },
    {   .uri   = "/restart",    
        .method    = HTTP_GET,
        .handler   = restart_get_handler,
        .user_ctx  = NULL,   
    },
    {   .uri   = "/favicon.ico",    
        .method    = HTTP_GET,
        .handler   = favicon_get_handler,
        .user_ctx  = NULL,   
    },
    {   .uri   = "/main.css",    
        .method    = HTTP_GET,
        .handler   = main_css_get_handler,
        .user_ctx  = NULL,   
    },
    {   .uri   = "/ajax.js",    
        .method    = HTTP_GET,
        .handler   = main_ajax_get_handler,
        .user_ctx  = NULL,   
    },
};

*/


//#endif /* __HTTP_HANDLERS_H__ */