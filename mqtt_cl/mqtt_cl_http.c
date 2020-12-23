#include "mqtt_cl_http.h"
#include "http_page_tpl.h"

#define HTML_PAGE_CFG_MQTT 2

#define URI_PARAM_MQTT_EN               "mqtt_en"
#define URI_PARAM_MQTT_HOST             "mqtt_host"
#define URI_PARAM_MQTT_LOGIN            "mqtt_login"
#define URI_PARAM_MQTT_PASSW            "mqtt_passw"
#define URI_PARAM_MQTT_TOPIC_BASE       "mqtt_base"
#define URI_PARAM_MQTT_SEND_INTERVAL    "mqtt_sint"

const char *html_page_title_mqtt ICACHE_RODATA_ATTR = "MQTT Settings";
const char *html_page_label_enable ICACHE_RODATA_ATTR = "Enabled";
const char *html_page_label_host ICACHE_RODATA_ATTR = "Hostname";
const char *html_page_label_login ICACHE_RODATA_ATTR = "Login";
const char *html_page_label_pass ICACHE_RODATA_ATTR = "Password";
const char *html_page_label_topic ICACHE_RODATA_ATTR = "Base topic (/)";
const char *html_page_label_interval ICACHE_RODATA_ATTR = "Send interval";

const char *html_page_config_mqtt ICACHE_RODATA_ATTR = "<div class='group rnd'>"
                                    "<h4 class='brd-btm'>MQTT Settings:</h4>"
                                    "<form method='GET'>"
                                        "<p><label class='lf'>Enabled: </label><input type='checkbox' name='"URI_PARAM_MQTT_EN"' %s /></p>"
                                        "<p><label class='lf'>Hostname: </label><input size='20' name='"URI_PARAM_MQTT_HOST"' class='edit rh' value='%s' /></p>"
                                        "<p><label class='lf'>Login: </label><input size='20' name='"URI_PARAM_MQTT_LOGIN"' class='edit rh' value='%s' /></p>"
                                        "<p><label class='lf'>Password: </label><input size='20' name='"URI_PARAM_MQTT_PASSW"' class='edit rh' value='%s' /></p>"
                                        "<p><label class='lf'>Base topic (/): </label><input size='20' name='"URI_PARAM_MQTT_TOPIC_BASE"' class='edit rh' value='%s' /></p>"
                                        "<p><label class='lf'>Send interval: </label><input size='20' name='"URI_PARAM_MQTT_SEND_INTERVAL"' class='edit rh' value='%d' /></p>"
                                        "<p><input type='hidden' name='st' value='2'></p>"  // HTML_PAGE_CFG_MQTT = 2
                                        "<p class='lf2'><input type='submit' value='Сохранить' class='button norm rht'></p>"
                                    "</form>"
                                    "</div>";  


static void mqtt_print_options(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    mqtt_config_t *mqtt_cfg = malloc(sizeof(mqtt_config_t));
    mqtt_get_cfg(mqtt_cfg);

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, html_page_title_mqtt);

    httpd_resp_sendstr_chunk(req, html_block_data_form_start);

    // ==========================================================================
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_checkbox
                                , html_page_label_enable // %s label
                                , URI_PARAM_MQTT_EN   // %s name
                                , mqtt_cfg->enabled  // %d value
                                , mqtt_cfg->enabled ? "checked" : ""
                                );

    // ==========================================================================
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                , html_page_label_host // %s label
                                , URI_PARAM_MQTT_HOST   // %s name
                                , mqtt_cfg->broker_url   // %d value
                                );

    // ==========================================================================
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                , html_page_label_login // %s label
                                , URI_PARAM_MQTT_LOGIN   // %s name
                                , mqtt_cfg->login   // %d value
                                );

    // ==========================================================================
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                , html_page_label_pass // %s label
                                , URI_PARAM_MQTT_PASSW   // %s name
                                , mqtt_cfg->password   // %d value
                                );

    // ==========================================================================
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                , html_page_label_topic // %s label
                                , URI_PARAM_MQTT_TOPIC_BASE   // %s name
                                , mqtt_cfg->base_topic   // %d value
                                );
    
    // ==========================================================================
    char *send_int = malloc(10);
    itoa(mqtt_cfg->send_interval, send_int, 10);
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                , html_page_label_interval // %s label
                                , URI_PARAM_MQTT_SEND_INTERVAL   // %s name
                                , send_int  // %d value
                                );

    free(send_int);
    free(mqtt_cfg);
    
    // ==========================================================================
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_submit, "2");

    // ==========================================================================
    httpd_resp_sendstr_chunk(req, html_block_data_form_end);
    httpd_resp_sendstr_chunk(req, html_block_data_end);
    httpd_resp_sendstr_chunk(req, html_block_data_end);
}

static void mqtt_print_debug(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    httpd_resp_sendstr_chunk_fmt(req, "<br>MQTT: <BR>"
    "status: %s<br>"
    "reconnects: %d<br>"
    , mqtt_state ? "connected" : "disconnected"
    , mqtt_reconnects
    ); 
}

void mqtt_register_http_print_data() 
{
    http_args_t *p = calloc(1,sizeof(http_args_t));
    register_print_page_block( "mqtt_options", PAGES_URI[ PAGE_URI_SETUP ], 2, mqtt_print_options, p, mqtt_http_process_params, NULL );
    register_print_page_block( "mqtt_debug", PAGES_URI[ PAGE_URI_DEBUG ], 2, mqtt_print_debug, p, NULL, NULL );
}

void mqtt_http_process_params(httpd_req_t *req, void *args)
{
   // check params

	if ( http_get_has_params(req) == ESP_OK) 
	{

        char param[100];
        mqtt_config_t *mqtt_cfg = malloc(sizeof(mqtt_config_t));
        // TODO: check for empty hostname and ssid
        if ( http_get_key_str(req, "st", param, sizeof(param)) == ESP_OK ) {
            if ( atoi(param) != HTML_PAGE_CFG_MQTT ) {
                return;	
            }
        }

        if ( http_get_key_str(req, URI_PARAM_MQTT_EN, param, sizeof(param)) == ESP_OK ) {
            mqtt_cfg->enabled = 1;
        } else {
            mqtt_cfg->enabled = 0;
        }

        if ( http_get_key_str(req, URI_PARAM_MQTT_HOST, param, sizeof(param)) == ESP_OK ) {
            url_decode(param, mqtt_cfg->broker_url);
        }


        if ( http_get_key_str(req, URI_PARAM_MQTT_LOGIN,  param, sizeof( param )) == ESP_OK ) {
            strcpy(mqtt_cfg->login, param);
        } 

        if ( http_get_key_str(req, URI_PARAM_MQTT_PASSW,  param, sizeof( param )) == ESP_OK ) {
            strcpy(mqtt_cfg->password, param);
        }

        if ( http_get_key_str(req, URI_PARAM_MQTT_TOPIC_BASE,  param, sizeof( param )) == ESP_OK ) {
            url_decode(param, mqtt_cfg->base_topic);
        }

        if ( http_get_key_str(req, URI_PARAM_MQTT_SEND_INTERVAL,  param, sizeof( param )) == ESP_OK ) {
            mqtt_cfg->send_interval = atoi(param);
        }

        
        mqtt_save_cfg(mqtt_cfg);
        free(mqtt_cfg);

        mqtt_restart_task();
    }
        
}

void mqtt_http_init(httpd_handle_t _server)
{
    mqtt_register_http_print_data();
}