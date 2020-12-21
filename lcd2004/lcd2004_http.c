#include "lcd2004_http.h"
#include "http_page_tpl.h"

#ifdef CONFIG_COMPONENT_LCD2004_HTTP


static const char *lcd2004_title ICACHE_RODATA_ATTR = "LCD2004";
static const char *lcd2004_param_addr ICACHE_RODATA_ATTR = "lcdaddr";
static const char *lcd2004_title_addr ICACHE_RODATA_ATTR = "Address";
static const char *param_st_lcd ICACHE_RODATA_ATTR = "lcd";
static const char *button_id ICACHE_RODATA_ATTR = "lcd%d";

const char *lcd2004_button_backlight_title ICACHE_RODATA_ATTR = "Подсветка";
const char *lcd2004_button_backlight_uri ICACHE_RODATA_ATTR = LCD2004_URI"?st=lcd&led=";

const char *lcd2004_button_clr_title ICACHE_RODATA_ATTR = "Очистить";
const char *lcd2004_button_clr_uri ICACHE_RODATA_ATTR = LCD2004_URI"?st=lcd&clr=";

const char *lcd2004_button_lcdon_uri ICACHE_RODATA_ATTR = LCD2004_URI"?st=lcd&on=";

void lcd2004_print_options(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    lcd2004_conf_t *cfg = (lcd2004_conf_t *)calloc(1, sizeof(lcd2004_conf_t));
    lcd2004_get_cfg( cfg );

    uint8_t state = lcd2004_backlight_state();
    uint8_t state2 = lcd2004_state();

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, lcd2004_title);

    httpd_resp_sendstr_chunk(req, html_block_data_form_start);
    httpd_resp_sendstr_chunk(req, html_block_data_div_lf3);

    // ==========================================================================
    char param[10];
    sprintf(param, "0x%02X", cfg->addr);
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit_hex
                                    , lcd2004_title_addr // %s label
                                    , lcd2004_param_addr   // %s name
                                    , param  // %d value
                                );
    
    httpd_resp_sendstr_chunk(req, html_block_data_end);

    // ==========================================================================
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_submit
                                    , param_st_lcd // %s st
                                );
    
    httpd_resp_sendstr_chunk(req, html_block_data_form_end);
    httpd_resp_sendstr_chunk(req, html_block_data_end);  
    httpd_resp_sendstr_chunk(req, "<div class='rht'>");
    // ==========================================================================    
    char rht[] = "rht";
    char *b_id = malloc( strlen(button_id) + 5);
    sprintf(b_id, button_id, 1);

    httpd_resp_sendstr_chunk_fmt(req, html_button
                                    , b_id
                                    , rht
                                    , "norm"
                                    , rht
                                    , lcd2004_button_clr_uri
                                    , !cfg->backlight
                                    , lcd2004_button_clr_title
                                    , 0
                                    , 1
                                    , lcd2004_button_clr_title
    );

    // ==========================================================================    
    sprintf(b_id, button_id, 2);
    
    httpd_resp_sendstr_chunk_fmt(req, html_button
                                    , b_id
                                    , rht
                                    , cfg->backlight ? " on" : " off"
                                    , rht
                                    , lcd2004_button_backlight_uri
                                    , !cfg->backlight
                                    , lcd2004_button_backlight_title
                                    , 0
                                    , 1
                                    , lcd2004_button_backlight_title
    );

    // ==========================================================================
    // TODO: переделать вывод кнопок через функцию   
    sprintf(b_id, button_id, 3);
  
    httpd_resp_sendstr_chunk_fmt(req, html_button
                                    , b_id
                                    , rht
                                    , cfg->backlight ? " on" : " off"
                                    , rht
                                    , lcd2004_button_backlight_uri
                                    , !cfg->backlight
                                    , "{0}"
                                    , 2
                                    , 1
                                    , cfg->state ? "ON" : "OFF"
    );

    free(b_id);
    free(cfg);
    httpd_resp_sendstr_chunk(req, html_block_data_end); 
    // ==========================================================================    
    httpd_resp_sendstr_chunk(req, html_block_data_end);    
}

void lcd2004_http_process_params(httpd_req_t *req, void *args)
{
    ESP_LOGI("LCD2004_HTTP", "%s", __func__);
   // check params
	if ( http_get_has_params(req) == ESP_OK) 
	{
        char param[100];
        if ( http_get_key_str(req, "st", param, sizeof(param)) == ESP_OK ) {
            if ( strcmp(param, param_st_lcd) != 0 ) {
                return;	
            }
        } 
        // TODO: обработать принятые данные  
        // опции дисплея    

        if ( http_get_key_str(req, "lcdaddr", param, sizeof(param)) == ESP_OK ) {
            ESP_LOGI("LCD2004_HTTP", "lcdaddr %s", param);
            lcd2004_conf_t *cfg = (lcd2004_conf_t *)calloc(1, sizeof(lcd2004_conf_t));
            lcd2004_get_cfg( cfg );
            cfg->addr = hex2int(param);     
            lcd2004_save_cfg(cfg);
            free(cfg);  
        } 
    } 
}

void lcd2004_register_http_print_data() 
{
    http_args_t *p = calloc(1,sizeof(http_args_t));
    register_print_page_block( "lcd2004_options", PAGES_URI[ PAGE_URI_TOOLS], 3, lcd2004_print_options, p, lcd2004_http_process_params, NULL );
}

esp_err_t lcd2004_get_handler(httpd_req_t *req)
{
    // check params
    char page[512] = ""; 
	if ( http_get_has_params(req) == ESP_OK) 
	{
        char param[100];
        if ( http_get_key_str(req, "st", param, sizeof(param)) == ESP_OK ) {
           
            if ( strcmp(param, param_st_lcd) != 0 ) {
                return ESP_FAIL;	
            }
        } 

        // peram led
        if ( http_get_key_str(req, "led", param, sizeof(param)) == ESP_OK ) 
        {
           uint8_t state = atoi(param);
            // turn on/off lcd led
            lcd2004_backlight( state );  
            state = lcd2004_backlight_state();  
            //itoa( state, page, 10);
            strcpy(page, state ? "ON" : "OFF");
            
            lcd2004_conf_t *cfg = (lcd2004_conf_t *)calloc(1, sizeof(lcd2004_conf_t));
            lcd2004_get_cfg( cfg );
            cfg->backlight = state;     
            lcd2004_save_cfg(cfg);
            free(cfg);             
        }  
        else if ( http_get_key_str(req, "clr", param, sizeof(param)) == ESP_OK ) 
        {
            // clear
            lcd2004_clear();  
            strcpy(page, "OK");
        }   
        else if ( http_get_key_str(req, "on", param, sizeof(param)) == ESP_OK ) 
        {
           uint8_t state = atoi(param);
            // turn on/off lcd state
            lcd2004_set_state( state );  
            state = lcd2004_state();  
            //itoa( state, page, 10);
            strcpy(page, state ? "ON" : "OFF");

            lcd2004_conf_t *cfg = (lcd2004_conf_t *)calloc(1, sizeof(lcd2004_conf_t));
            lcd2004_get_cfg( cfg );
            cfg->state = state;     
            lcd2004_save_cfg(cfg);
            free(cfg);                        
        }       
        else 
        {
            return ESP_FAIL;
        }
    } 

    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
	httpd_resp_send(req, page, strlen(page)); 
     
    return ESP_OK;
}

void lcd2004_register_http_handler(httpd_handle_t _server)
{
    add_uri_get_handler( _server, LCD2004_URI, lcd2004_get_handler, NULL); 
}

void lcd2004_http_init(httpd_handle_t _server)
{
    lcd2004_register_http_print_data();
    lcd2004_register_http_handler(_server);
}

#endif