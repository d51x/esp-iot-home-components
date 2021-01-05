#include "httpd.h"
#include "http_handlers.h"


static const char *TAG = "HTTPD";

uint8_t http_handlers_count = PAGE_URI_MAX + 1;
/********************* Basic Handlers Start *******************/




void register_uri_handlers(httpd_handle_t _server) {
    
    add_uri_get_handler( _server, PAGES_URI[PAGE_URI_ROOT], main_get_handler, &PAGES_HANDLER[PAGE_URI_ROOT]); 
    add_uri_get_handler( _server, PAGES_URI[PAGE_URI_SETUP], setup_get_handler, &PAGES_HANDLER[PAGE_URI_SETUP]); 
    
    #ifdef CONFIG_PAGE_DEBUG
    add_uri_get_handler( _server, PAGES_URI[PAGE_URI_DEBUG], debug_get_handler, &PAGES_HANDLER[PAGE_URI_DEBUG]); 
    #endif

    add_uri_get_handler( _server, PAGES_URI[PAGE_URI_CONFIG], config_get_handler, &PAGES_HANDLER[PAGE_URI_CONFIG]); 
    
    #ifdef CONFIG_PAGE_TOOLS
    add_uri_get_handler( _server, PAGES_URI[PAGE_URI_TOOLS], tools_get_handler, &PAGES_HANDLER[PAGE_URI_TOOLS]); 
    #endif
    
    add_uri_get_handler( _server, PAGES_URI[PAGE_URI_OTA], update_get_handler, &PAGES_HANDLER[PAGE_URI_OTA]); 

    add_uri_get_handler( _server, PAGES_URI[PAGE_URI_REBOOT], reboot_get_handler, NULL); 
    add_uri_get_handler( _server, PAGES_URI[PAGE_URI_FAVICO], favicon_get_handler, NULL); 
    add_uri_get_handler( _server, PAGES_URI[PAGE_URI_CSS], main_css_get_handler, NULL); 
    add_uri_get_handler( _server, PAGES_URI[PAGE_URI_AJAX], main_ajax_get_handler, NULL); 

    add_uri_get_handler( _server, "/menu.png", icons_get_handler, NULL); 
    add_uri_get_handler( _server, "/menu2.png", icons_get_handler, NULL); 

}

void add_uri_get_handler(httpd_handle_t _server, const char *uri, httpd_uri_func func, void *ctx) {

    //user_ctx_t *_ctx = (user_ctx_t *) ctx;

    httpd_uri_t my_uri;
      my_uri.uri      = strdup(uri);
      my_uri.method   = HTTP_GET;
      my_uri.handler  = func;


    if ( ctx != NULL ) 
    {
      my_uri.user_ctx = ctx;
    }

    /*esp_err_t err =*/ httpd_register_uri_handler(_server, &my_uri);
}


void add_uri_post_handler(httpd_handle_t _server, const char *uri, httpd_uri_func func, void *ctx) 
{
    //user_ctx_t *_ctx = (user_ctx_t *) ctx;

    httpd_uri_t my_uri;
      my_uri.uri      = strdup(uri);
      my_uri.method   = HTTP_POST;
      my_uri.handler  = func;
      my_uri.user_ctx = NULL;

    if ( ctx != NULL ) 
    {
      my_uri.user_ctx = ctx;
    }

    /*esp_err_t err =*/ httpd_register_uri_handler(_server, &my_uri);

}

void webserver_init(httpd_handle_t* _server) {
    /* Start the web server */
    if ( *_server == NULL) {
        *_server = webserver_start();
    }
}



httpd_handle_t webserver_start(void)
{
    httpd_handle_t _server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = WEB_SERVER_STACK_SIZE;

    //config.max_uri_handlers = WEB_SERVER_MAX_URI_HANDLERS; //100; //uri_handlers_no; //WEB_SERVER_MAX_URI_GET_HANDLERS;
    config.max_uri_handlers = http_handlers_count;
    config.recv_wait_timeout = 10;   
    config.lru_purge_enable = true;  /* If no space is available for new session, close the least recently used one */
    
    // Start the httpd server
    ESP_LOGW(TAG, "******** Starting server on port: '%d'", config.server_port);
    if (httpd_start(&_server, &config) == ESP_OK) {
        // Set URI handlers
        register_uri_handlers(_server);
        ESP_LOGW(TAG, "http_handlers_count = %d", http_handlers_count );

        page_initialize_menu();
        return _server;
    }
    ESP_LOGE(TAG, LOG_FMT("server not started"));
    return NULL;
}

void webserver_stop(httpd_handle_t _server){
    // Stop the httpd server
    httpd_stop(_server);
}


esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

void make_redirect(httpd_req_t *req, uint8_t timeout, const char *path) {
    char t[4];
    itoa(timeout, t, 10);
    char *hdr = calloc(1, strlen(t) + 2 + strlen(path) + 1);
    strcpy(hdr, t);
    strcat(hdr, "; ");
    strcat(hdr, path);
    httpd_resp_set_hdr(req, "Refresh", hdr);
    httpd_resp_send(req, NULL, 0);
    free(hdr);
}