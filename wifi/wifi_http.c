#include "wifi_http.h"
#include "http_page_tpl.h"

#define HTML_PAGE_CFG_WIFI 1

#define URI_PARAM_WIFI_HOST             "hostname"
#define URI_PARAM_WIFI_SSID             "ssid"
#define URI_PARAM_WIFI_PASSW            "pass"
#define URI_PARAM_WIFI_MODE             "wifi_mode"
#define URI_PARAM_WIFI_IP_ADDR             "ipaddr"

const char *html_page_config_wifi_title ICACHE_RODATA_ATTR = "WiFi Settings";
const char *html_page_config_wifi_hostname ICACHE_RODATA_ATTR = "Hostname";
const char *html_page_config_wifi_ssid ICACHE_RODATA_ATTR = "SSID";
const char *html_page_config_wifi_pass ICACHE_RODATA_ATTR = "Password";

const char *html_page_config_wifi_mode ICACHE_RODATA_ATTR = 

			"<p><label class='lf'>Mode:</label>"
				"<label><input type='radio' name='"URI_PARAM_WIFI_MODE"' value='1' %s /> STA</label>" 
				"<label><input type='radio' name='"URI_PARAM_WIFI_MODE"' value='2' %s /> AP</label>"
			"</p>"
;

const char *html_page_config_wifi_ip_addr ICACHE_RODATA_ATTR = 

			"<p><label class='lf'>IP address:</label>"
				"<input size='20' name='"URI_PARAM_WIFI_IP_ADDR"' class='edit rh' value='%s' %s />"
			"</p>"
;

static void wifi_print_options(http_args_t *args)
{
	http_args_t *arg = (http_args_t *)args;
	httpd_req_t *req = (httpd_req_t *)arg->req;

	httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, html_page_config_wifi_title);

    httpd_resp_sendstr_chunk(req, html_block_data_form_start);

	// ==========================================================================
	httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                , html_page_config_wifi_hostname // %s label
                                , URI_PARAM_WIFI_HOST   // %s name
                                , wifi_cfg->hostname   // %d value
                                );
	// ==========================================================================
	httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                , html_page_config_wifi_ssid // %s label
                                , URI_PARAM_WIFI_SSID   // %s name
                                , wifi_cfg->ssid   // %d value
                                );
	// ==========================================================================
	httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                , html_page_config_wifi_pass // %s label
                                , URI_PARAM_WIFI_PASSW   // %s name
                                , wifi_cfg->password   // %d value
                                );

	// ==========================================================================
	httpd_resp_sendstr_chunk_fmt(req, html_page_config_wifi_mode
											, (wifi_cfg->mode == WIFI_MODE_STA) ? "checked" : ""          // sta checked
										, (wifi_cfg->mode == WIFI_MODE_AP)  ? "checked" : ""         // ap checked
                                );

	// ==========================================================================
	httpd_resp_sendstr_chunk_fmt(req, html_page_config_wifi_ip_addr, wifi_cfg->ip, "disabled");

	// ==========================================================================
	httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_submit, "1");

	// ==========================================================================
    httpd_resp_sendstr_chunk(req, html_block_data_form_end);
    httpd_resp_sendstr_chunk(req, html_block_data_end);
    httpd_resp_sendstr_chunk(req, html_block_data_end);

}

static void wifi_print_debug(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;
    httpd_resp_sendstr_chunk_fmt(req, "<br>Wifi reconnects: %d<br>", wifi_get_reconnect_count());
}

void wifi_register_http_print_data() 
{
	http_args_t *p = calloc(1,sizeof(http_args_t));
	register_print_page_block( "wifi_options", PAGES_URI[ PAGE_URI_SETUP ], 1, wifi_print_options, p, (httpd_uri_func) wifi_http_process_params, NULL );
	register_print_page_block( "wifi_dbg", PAGES_URI[ PAGE_URI_DEBUG ], 1, wifi_print_debug, p, NULL, NULL );
}


void wifi_http_process_params(httpd_req_t *req, void *args)
{
	 // check params

	char param[100];
	// TODO: check for empty hostname and ssid
	if ( http_get_key_str(req, "st", param, sizeof(param)) == ESP_OK ) {
		if ( atoi(param) != HTML_PAGE_CFG_WIFI ) {
			return;	
		}
	}

 	if ( http_get_key_str(req, URI_PARAM_WIFI_HOST, param, sizeof(param)) == ESP_OK ) {
				strcpy(wifi_cfg->hostname, param);
				tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, wifi_cfg->hostname);
				
				mqtt_set_device_name(wifi_cfg->hostname);
		}

		if ( http_get_key_str(req, URI_PARAM_WIFI_SSID, param, sizeof(param)) == ESP_OK ) {
				strcpy(wifi_cfg->ssid, param);
		}

		if ( http_get_key_str(req, URI_PARAM_WIFI_PASSW, param, sizeof(param)) == ESP_OK ) {
				strcpy(wifi_cfg->password, param);
		}

		wifi_cfg->mode = WIFI_MODE_NULL;
		if ( http_get_key_str(req, URI_PARAM_WIFI_MODE, param, sizeof(param)) == ESP_OK ) {
				wifi_cfg->mode = atoi(param);
		}

	wifi_cfg->first = 0;
	wifi_cfg_save(wifi_cfg);
}

void wifi_http_init(httpd_handle_t _server)
{
	wifi_register_http_print_data();
}