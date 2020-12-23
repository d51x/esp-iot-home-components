#include "relay_http.h"
#include "http_page_tpl.h"

#ifdef CONFIG_RELAY_HTTP

static const char* TAG = "RELAY";

static const char *relay_block_title ICACHE_RODATA_ATTR = "Relays"; 
static const char *button_id ICACHE_RODATA_ATTR = "relay%d"; 
static const char *button_uri ICACHE_RODATA_ATTR = RELAY_URI "?pin=%d&st="; 

#ifdef CONFIG_RELAY_CONFIG
const char *html_page_title_relays_cfg ICACHE_RODATA_ATTR = "Relays Config";
#endif

void relay_print_button(httpd_req_t *req, const char *btn_id, uint8_t idx)
{
        char *b_uri = malloc( strlen(button_uri) + 5);
        sprintf(b_uri, button_uri, relays[idx].pin );
    
        httpd_resp_sendstr_chunk_fmt(req, html_button
                                        , btn_id
                                        , "lht"
                                        , relays[idx].state ? "on" : "off"
                                        , "lht"
                                        , b_uri
                                        , !relays[idx].state
                                        , relays[idx].name
                                        , 0
                                        , 1
                                        , relays[idx].name
        );
        free(b_uri);                           
}

static void relay_print_data_main(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    if ( relay_count == 0 ) return;

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, relay_block_title);

    for (uint8_t i = 0; i < relay_count; i++)
    {
        char *b_id = malloc( strlen(button_id) + 5);
        sprintf(b_id, button_id, relays[i].pin );
        relay_print_button(req, b_id, i);
        free(b_id);
    }

    
    httpd_resp_sendstr_chunk(req, html_block_data_end);    
}

#ifdef CONFIG_RELAY_CONFIG
static void relay_print_data_conf(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, html_page_title_relays_cfg);

    httpd_resp_sendstr_chunk(req, html_block_data_end);  
}
#endif

static void relay_register_http_print_data() 
{
    http_args_t *p = calloc(1,sizeof(http_args_t));
    register_print_page_block( "relay_main", PAGES_URI[ PAGE_URI_ROOT], 3, relay_print_data_main, p, NULL, NULL );
    #ifdef CONFIG_RELAY_CONFIG
    register_print_page_block( "relay_conf", RELAY_URI, 2, relay_print_data_conf, p, NULL, NULL); 
    #endif
}

static esp_err_t relay_get_handler(httpd_req_t *req)
{
    // check params
    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
    char page[100] = ""; 
    esp_err_t err = ESP_FAIL;
	if ( http_get_has_params(req) == ESP_OK) 
	{

        /*
            <ip>/relay?pin=12&st=1
            <ip>/relay?pin=12
            <ip>/relay?all=1
        */

        char param[20];
        if ( http_get_key_str(req, "pin", param, sizeof(param)) == ESP_OK ) 
        {
            uint8_t pin = atoi(param);
            if ( http_get_key_str(req, "st", param, sizeof(param)) == ESP_OK )
            {
                uint8_t st = atoi(param);
                for (uint8_t i = 0; i < relay_count; i++)
                {
                    if ( relays[i].pin == pin )
                    {
                        err =  relay_write( (relay_handle_t)&relays[i], st);
                        itoa(st, param, 10);
                        httpd_resp_sendstr_chunk(req, param);
                        break;
                    }
                }
                
            }
            else
            {
                for (uint8_t i = 0; i < relay_count; i++)
                {
                    if ( relays[i].pin == pin )
                    {
                        relay_state_t st =  relay_read( (relay_handle_t)&relays[i]);
                        itoa(st, param, 10);
                        httpd_resp_sendstr_chunk(req, param);
                        break;
                    }
                }             
            }            
        }
        else if ( http_get_key_str(req, "all", param, sizeof(param)) == ESP_OK ) 
        {
            httpd_resp_sendstr_chunk(req, "{");
            for (uint8_t i = 0; i < relay_count; i++)
            {
                relay_state_t st =  relay_read( (relay_handle_t)&relays[i]);
                char *buf = malloc(20);
                sprintf(buf, "\"relay%d\": %d",  relays[i].pin, st );
                httpd_resp_sendstr_chunk(req, buf);
                if ( i < relay_count-1 )
                    httpd_resp_sendstr_chunk(req, ", ");
            }
            httpd_resp_sendstr_chunk(req, "}");
        }
        else
        {
            httpd_resp_sendstr_chunk(req, html_error);
        }
    } else {
        // print relays data
        // кнопка "добавить"
        // поле для имени
        // поле для gpio
        // checkbox для указания инвертирования
        // checkbox для указания сохранения nvs
        show_http_page( req );
    }

    //httpd_resp_send_chunk(req, NULL, 0); 
    httpd_resp_end(req);
    return ESP_OK;    
}

static void relay_register_http_handler(httpd_handle_t _server)
{   
    user_ctx_t *ctx = NULL;
    #ifdef CONFIG_RELAY_CONFIG
        ctx = (user_ctx_t *)calloc(1, sizeof(user_ctx_t));
        strncpy(ctx->title, "Relays", 20);
        ctx->show = true; 
    #endif
    
    add_uri_get_handler( _server, RELAY_URI, relay_get_handler, ctx); 
    // еще зарегистрировать post handler для сохранения данных в nvs 
}

void relay_http_init(httpd_handle_t _server)
{
    relay_register_http_print_data();  
    relay_register_http_handler(_server);
    
    #ifdef CONFIG_RELAY_CONFIG
    register_http_page_menu( RELAY_URI, relay_block_title);
    #endif
}

#endif
