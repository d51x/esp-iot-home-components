

#include "sht21_http.h"
#include "http_page_tpl.h"

#ifdef CONFIG_SENSOR_SHT21
static const char *TAG = "SHT21";

const char *html_block_sht21_title ICACHE_RODATA_ATTR = "SHT21 sensor";
const char *html_block_sht21_title_temp ICACHE_RODATA_ATTR = "Temperature";
const char *html_block_sht21_title_hum ICACHE_RODATA_ATTR = "Humidity";

static void sht21_print_data(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, html_block_sht21_title);

    // ==========================================================================
    char param[10];
    sprintf(param, "%0.2f°C", sht21_get_temp());

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_label
                                    , html_block_sht21_title_temp // %s label
                                    , param   // %s name
                                );

    // ==========================================================================
    sprintf(param, "%0.2f%%", sht21_get_hum());

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_label
                                    , html_block_sht21_title_hum // %s label
                                    , param   // %s name
                                );

    // ==========================================================================
    httpd_resp_sendstr_chunk(req, html_block_data_end); 
}

void sht21_register_http_print_data() 
{
    http_args_t *p = calloc(1,sizeof(http_args_t));
    register_print_page_block( "sht21_data", PAGES_URI[ PAGE_URI_ROOT], 3, sht21_print_data, p, NULL, NULL);
    
}

void sht21_http_init(httpd_handle_t _server)
{
    sht21_register_http_print_data();
}
#endif

