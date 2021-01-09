
#include "http_handlers.h"
#include "http_page.h"
#include "httpd.h"

/*
pages:

  menu: 
      main - main page
      setup - auth, wifi type and auth for sta 
*/


static const char *TAG = "HTTPH";

const char *PAGES_URI[PAGE_URI_MAX] = { 
    HTTP_URI_ROOT,                    // PAGE_URI_ROOT
    HTTP_URI_SETUP,               // PAGE_URI_SETUP
    
    #ifdef CONFIG_PAGE_DEBUG
    HTTP_URI_DEBUG,               // PAGE_URI_DEBUG
    #endif

    HTTP_URI_CONFIG,               // PAGE_URI_DEBUG
    
    #ifdef CONFIG_PAGE_TOOLS
    HTTP_URI_TOOLS,               // PAGE_URI_TOOLS
    #endif

    HTTP_URI_OTA,              // PAGE_URI_OTA
    HTTP_URI_REBOOT,              // PAGE_URI_REBOOT
    "/main.css",            // PAGE_URI_CSS
    "/ajax.js",             // PAGE_URI_AJAX
    #ifdef CONFIG_SENSOR_MQTT
    "/mqtt.js",             
    #endif           
    #ifdef CONFIG_RGB_CONTROLLER
    "/rgb.js",             
    #endif
    "/favicon.ico",         // PAGE_URI_FAVICO
    HTTP_URI_ICON_MENU,            // PAGE_URI_ICON_MENU
    HTTP_URI_ICON_MENU2            // PAGE_URI_ICON_MENU2
    };



user_ctx_t PAGES_HANDLER[PAGE_URI_MAX] = {
     {HTTP_STR_MAIN,    true,   show_page_main,       NULL}
    ,{HTTP_STR_SETUP,   true,   show_page_setup,      NULL}

    #ifdef CONFIG_PAGE_DEBUG
    ,{HTTP_STR_DEBUG,   true,   show_page_debug,      NULL}
    #endif

    ,{HTTP_STR_CONFIG,   true,   show_page_config,      NULL}

    #ifdef CONFIG_PAGE_TOOLS
    ,{HTTP_STR_TOOLS,   true,   show_page_tools,      NULL}
    #endif

    ,{HTTP_STR_OTA,     true,   show_page_ota,     NULL}
    ,{HTTP_STR_REBOOT,  false,  reboot_get_handler,   NULL}
};

http_menu_item_t *http_menu = NULL;

uint8_t menu_items_count = MENU_ITEM_COUNT;


void process_params(httpd_req_t *req)
{
    for (uint8_t i = 0; i < http_print_page_block_count; i++)
    {
        char *_uri;
        _uri = http_uri_clean(req);
        if ( strcmp( _uri, http_print_page_block[i].uri) == 0 && http_print_page_block[i].process_cb != NULL) {
            // do
            http_print_page_block[i].process_cb(req, http_print_page_block[i].args2); 
        }
        free(_uri);
    }
}

esp_err_t main_get_handler(httpd_req_t *req) 
{
    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
    show_http_page( req );
    //httpd_resp_send_chunk(req, NULL, 0);
    httpd_resp_end(req);

    #ifdef CONFIG_COMPONENT_DEBUG
        print_task_stack_depth(TAG, "httpd task");
    #endif

    return ESP_OK;
}

esp_err_t setup_get_handler(httpd_req_t *req){

  // check params
  httpd_resp_set_type(req, HTTPD_TYPE_TEXT);

	if ( http_get_has_params(req) == ESP_OK) 
	{
        process_params(req);

        char *path = http_uri_clean( req );
        make_redirect(req, 0, path);
        free( path );
        return ESP_OK;
	}
	  
  //show_page_setup( page );
  show_http_page( req );
  
  //httpd_resp_send_chunk(req, NULL, 0);
  httpd_resp_end(req);
  return ESP_OK;
}

esp_err_t config_get_handler(httpd_req_t *req){
    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);

  // check params
	if ( http_get_has_params(req) == ESP_OK) 
	{
        process_params(req);

        char *path = http_uri_clean( req );
        make_redirect(req, 0, path);
        free( path );
        return ESP_OK;
	}

  show_http_page( req );
  //httpd_resp_send_chunk(req, NULL, 0);
  httpd_resp_end(req);
  return ESP_OK;
}

#ifdef CONFIG_PAGE_TOOLS
esp_err_t tools_get_handler(httpd_req_t *req)
{

  // check params
	if ( http_get_has_params(req) == ESP_OK) 
	{
        process_params(req);

        char *path = http_uri_clean( req );
        make_redirect(req, 0, path);
        free( path );
        return ESP_OK;        
	}
  
  httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
  show_http_page( req );
  
  //httpd_resp_send_chunk(req, NULL, 0);
  httpd_resp_end(req);
  return ESP_OK;
}
#endif

esp_err_t update_get_handler(httpd_req_t *req){

	if ( http_get_has_params(req) == ESP_OK) 
	{
        process_params(req);
        return ESP_OK; 
	}

  httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
  show_http_page( req );

  //httpd_resp_send_chunk(req, NULL, 0);
  httpd_resp_end(req);
  return ESP_OK;
}

#ifdef CONFIG_PAGE_DEBUG
esp_err_t debug_get_handler(httpd_req_t *req){
  httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
  show_http_page( req );

  //httpd_resp_send_chunk(req, NULL, 0);
  httpd_resp_end(req);
  return ESP_OK;
}
#endif

esp_err_t reboot_get_handler(httpd_req_t *req)
{
	uint8_t found = 0;
	
	if ( http_get_has_params(req) == ESP_OK) {
        uint8_t st;
        if ( http_get_key_uint8(req, "st", &st, 0) == ESP_OK ) {
            found = (st == 1);        
        }
    } 

    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);

    if ( found ) {  
        xTaskCreate(systemRebootTask, "systemRebootTask", 1024, 2000, 5, NULL);
        httpd_resp_set_hdr(req, "Refresh", "5; /");
        httpd_resp_send(req, NULL, 0);
    
    } else {
		httpd_resp_sendstr_chunk (req, "Please restart ESP"); 
    }
    //httpd_resp_send_chunk(req, NULL, 0); 
    httpd_resp_end(req);
    return ESP_OK;
}


esp_err_t favicon_get_handler(httpd_req_t *req)
{
    extern const unsigned char favicon_ico_start[] asm("_binary_favicon_ico_start");
    extern const unsigned char favicon_ico_end[]   asm("_binary_favicon_ico_end");
    const size_t favicon_ico_size = (favicon_ico_end - favicon_ico_start);
    httpd_resp_set_type(req, HTTP_MEDIA_TYPE_ICON);
    httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_size);
    return ESP_OK;
}

esp_err_t icons_get_handler(httpd_req_t *req)
{
    size_t icon_size = 0;
    char *icon_start = NULL;

	if ( strcmp(req->uri, HTTP_URI_ICON_MENU) == ESP_OK) 
    {
        icon_size = (menu_png_end - menu_png_start);
        icon_start = (char *)menu_png_start;
    }
    else if ( strcmp(req->uri, HTTP_URI_ICON_MENU2) == ESP_OK) 
    {
        icon_size = (menu2_png_end - menu2_png_start);
        icon_start = (char *)menu2_png_start;
    }        

    httpd_resp_set_type(req, HTTP_MEDIA_TYPE_ICON);
    httpd_resp_send(req, (const char *)icon_start, icon_size);
    return ESP_OK;
}


esp_err_t main_css_get_handler(httpd_req_t *req)
{
    extern const unsigned char main_css_start[] asm("_binary_main_min_css_start");
    extern const unsigned char main_css_end[]   asm("_binary_main_min_css_end");
    const size_t main_css_size = (main_css_end - main_css_start);
    
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_hdr(req, "Pragma", "no-cache");
    httpd_resp_set_hdr(req, "Cache-control", "no-cache");

    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char *)main_css_start, main_css_size);
    return ESP_OK;
}

esp_err_t main_ajax_get_handler(httpd_req_t *req)
{
    extern const unsigned char ajax_js_start[] asm("_binary_ajax_min_js_start");
    extern const unsigned char ajax_js_end[]   asm("_binary_ajax_min_js_end");
    const size_t ajax_js_size = (ajax_js_end - ajax_js_start);
    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_send(req, (const char *)ajax_js_start, ajax_js_size);
    return ESP_OK;
}

#ifdef CONFIG_SENSOR_MQTT
esp_err_t main_mqtt_js_get_handler(httpd_req_t *req)
{
    extern const unsigned char mqtt_js_start[] asm("_binary_mqtt_min_js_start");
    extern const unsigned char mqtt_js_end[]   asm("_binary_mqtt_min_js_end");
    const size_t mqtt_js_size = (mqtt_js_end - mqtt_js_start);
    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_send(req, (const char *)mqtt_js_start, mqtt_js_size);
    return ESP_OK;
}
#endif

#ifdef CONFIG_RGB_CONTROLLER
esp_err_t main_rgb_js_get_handler(httpd_req_t *req)
{
    extern const unsigned char rgb_js_start[] asm("_binary_rgb_min_js_start");
    extern const unsigned char rgb_js_end[]   asm("_binary_rgb_min_js_end");
    const size_t rgb_js_size = (rgb_js_end - rgb_js_start);
    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_send(req, (const char *)rgb_js_start, rgb_js_size);
    return ESP_OK;
}
#endif

// TODO show custom page handler from component
//show_http_page( req, page);

esp_err_t register_http_process_page_data(const char *uri, httpd_uri_func fn_cb)
{
    return ESP_OK;
}