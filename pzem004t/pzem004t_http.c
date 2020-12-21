#include "pzem004t_http.h"
#include "http_page_tpl.h"
#include "http_page.h"

#ifdef CONFIG_SENSOR_PZEM004_T_WEB

    static const char *TAG = "PZEM";


const char *html_block_pzem004t_title ICACHE_RODATA_ATTR = "PZEM-004T";
const char *html_block_pzem004t_title_voltage ICACHE_RODATA_ATTR = "Voltage";
const char *html_block_pzem004t_title_current ICACHE_RODATA_ATTR = "Current";
const char *html_block_pzem004t_title_power ICACHE_RODATA_ATTR = "Power";
const char *html_block_pzem004t_title_energy_total ICACHE_RODATA_ATTR = "Energy total";
const char *html_block_pzem004t_title_energy_today ICACHE_RODATA_ATTR = "Energy today";
const char *html_block_pzem004t_title_energy_yesterday ICACHE_RODATA_ATTR = "Energy yesterday";
const char *html_block_pzem004t_title_errors ICACHE_RODATA_ATTR = "Errors";

#ifdef CONFIG_SENSOR_PZEM004_T_CALC_CONSUMPTION
const char *html_block_pzem004t_title_consump_today ICACHE_RODATA_ATTR = "Расход сегодня";
const char *html_block_pzem004t_title_consump_prev ICACHE_RODATA_ATTR = "Расход вчера";

const char *html_block_pzem004t_title_consump_day ICACHE_RODATA_ATTR = "день";
const char *html_block_pzem004t_title_consump_night ICACHE_RODATA_ATTR = "ночь";

const char *html_block_pzem004t_title_energy_now ICACHE_RODATA_ATTR = "Сейчас";
const char *html_block_pzem004t_title_energy_midnight_today ICACHE_RODATA_ATTR = "Сегодня в 00:00";
const char *html_block_pzem004t_title_energy_t1_today ICACHE_RODATA_ATTR = "Сегодня в 07:00";
const char *html_block_pzem004t_title_energy_t2_today ICACHE_RODATA_ATTR = "Сегодня в 23:00";
const char *html_block_pzem004t_title_energy_midnight_prev ICACHE_RODATA_ATTR = "Вчера в 00:00";
const char *html_block_pzem004t_title_energy_t1_prev ICACHE_RODATA_ATTR = "Вчера в 07:00";
const char *html_block_pzem004t_title_energy_t2_prev ICACHE_RODATA_ATTR = "Вчера в 23:00";

#define PZEM_URI "/pzem"
const char *pzem_reset_uri ICACHE_RODATA_ATTR = PZEM_URI"?reset=1";
const char *pzem_reset_today_uri ICACHE_RODATA_ATTR = PZEM_URI"?reset=1&today=1";
const char *pzem_reset_title ICACHE_RODATA_ATTR = "Сброс расхода";

#endif



static void pzem_print_options(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, html_block_pzem004t_title);

    // httpd_resp_sendstr_chunk(req, html_block_data_form_start);
    // httpd_resp_sendstr_chunk(req, html_block_data_div_lf3);
    httpd_resp_sendstr_chunk(req, "<div class='rht'>");
    // ==============================================
    // print page block data here
    // ==============================================

    // сброс вычисленного расхода только сегодня
    //http_print_button(req, "pzemrst1", "rht", "norm", pzem_reset_uri, -1, pzem_reset_title, 0, 0);

    // сброс вычисленного расхода вчера и сегодня
    http_print_button(req, "pzemrst2", "rht", "norm", pzem_reset_today_uri, -1, pzem_reset_title, 0, 0);

    // сброс вычисленного расхода только вчера
    // TODO
    
    // ==============================================
    
    httpd_resp_sendstr_chunk(req, html_block_data_end); 
    httpd_resp_sendstr_chunk(req, html_block_data_end);   
}

static void pzem_print_debug(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;
    pzem_data_t pzem_data = pzem_get_data();
    httpd_resp_sendstr_chunk_fmt(req, "PZEM %s: %d", html_block_pzem004t_title_errors, pzem_data.errors);
    //http_print_value(req, html_block_data_form_item_label_label, html_block_pzem004t_title_errors, "%d", UINT16_T, (void *)&pzem_data.errors);
}

static void pzem_print_data(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, html_block_pzem004t_title);

    pzem_data_t pzem_data = pzem_get_data();

    http_print_value(req, html_block_data_form_item_label_label, html_block_pzem004t_title_voltage, "%0.2f V", TYPE_FLOAT, (void *)&pzem_data.voltage);

    http_print_value(req, html_block_data_form_item_label_label, html_block_pzem004t_title_current, "%0.2f A", TYPE_FLOAT, (void *)&pzem_data.current);

    float t = pzem_data.power / PZEM_FLOAT_DIVIDER;
    http_print_value(req, html_block_data_form_item_label_label, html_block_pzem004t_title_power, "%0.2f kW/h", TYPE_FLOAT, (void *)&t);

    t = pzem_data.energy / PZEM_FLOAT_DIVIDER;
    http_print_value(req, html_block_data_form_item_label_label, html_block_pzem004t_title_energy_total, "%0.2f kW*h", TYPE_FLOAT, (void *)&t);

    #ifdef CONFIG_SENSOR_PZEM004_T_CALC_CONSUMPTION
    // ==========================================================================
    // расход вчера: общий
    t = pzem_data.consumption.prev_total / PZEM_FLOAT_DIVIDER;
    http_print_value(req, html_block_data_form_item_label_label, html_block_pzem004t_title_consump_prev, "%0.2f kW*h", TYPE_FLOAT, (void *)&t);

    if ( PZEM_ENERGY_ZONE_T1_HOUR > 0 && PZEM_ENERGY_ZONE_T2_HOUR > 0 )
    {
        // расход вчера: ночь
        t = pzem_data.consumption.prev_night / PZEM_FLOAT_DIVIDER;
        http_print_value(req, html_block_data_form_item_label_label, html_block_pzem004t_title_consump_night, "%0.2f kW*h", TYPE_FLOAT, (void *)&t);
        
        // расход вчера: день
        t = pzem_data.consumption.prev_day / PZEM_FLOAT_DIVIDER;
        http_print_value(req, html_block_data_form_item_label_label, html_block_pzem004t_title_consump_day, "%0.2f kW*h", TYPE_FLOAT, (void *)&t);        
    }

    // ==========================================================================
    // расход сегодня: общий
    t = pzem_data.consumption.today_total / PZEM_FLOAT_DIVIDER;
    http_print_value(req, html_block_data_form_item_label_label, html_block_pzem004t_title_consump_today, "%0.2f kW*h", TYPE_FLOAT, (void *)&t); 

    if ( PZEM_ENERGY_ZONE_T1_HOUR > 0 && PZEM_ENERGY_ZONE_T2_HOUR > 0 )
    {
        // расход сегодня: ночь
        t = pzem_data.consumption.today_night / PZEM_FLOAT_DIVIDER;
        http_print_value(req, html_block_data_form_item_label_label, html_block_pzem004t_title_consump_night, "%0.2f kW*h", TYPE_FLOAT, (void *)&t); 

        // расход сегодня: день
        t = pzem_data.consumption.today_day / PZEM_FLOAT_DIVIDER;
        http_print_value(req, html_block_data_form_item_label_label, html_block_pzem004t_title_consump_day, "%0.2f kW*h", TYPE_FLOAT, (void *)&t); 

    }

    #endif

    httpd_resp_sendstr_chunk(req, html_block_data_end); 
}

static void pzem_http_process_params(httpd_req_t *req, void *args)
{
   // check params
	if ( http_get_has_params(req) == ESP_OK) 
	{
        char param[100];
        if ( http_get_key_str(req, "st", param, sizeof(param)) == ESP_OK ) {
            //if ( strcmp(param, param_st_lcd) != 0 ) {
            //    return;	
            //}
        } 
        // TODO: обработать принятые данные  

    } 
}

void pzem_register_http_print_data()
{
    http_args_t *p = calloc(1,sizeof(http_args_t));
    register_print_page_block( "pzem_data", PAGES_URI[ PAGE_URI_ROOT], 3, pzem_print_data, p, NULL, NULL);    
    register_print_page_block( "pzem_tools", PAGES_URI[ PAGE_URI_TOOLS], 3, pzem_print_options, p, pzem_http_process_params, NULL);    
    register_print_page_block( "pzem_debug", PAGES_URI[ PAGE_URI_DEBUG], 3, pzem_print_debug, p, NULL, NULL);    
}

static esp_err_t pzem_get_handler(httpd_req_t *req)
{
    // check params
    char page[10] = ""; 
	if ( http_get_has_params(req) == ESP_OK) 
	{
        char param[20];
        //  /pzem?reset=1&today=1
        if ( http_get_key_str(req, "reset", param, sizeof(param)) == ESP_OK ) {
            uint8_t state = atoi(param);
            if ( state ) {
                state = 0;
                if ( http_get_key_str(req, "today", param, sizeof(param)) == ESP_OK ) {
                    state = atoi(param);
                }
                pzem_reset_consumption(state);
                strcpy(page, "OK");
            }
        } else {
            strcpy(page, "ERROR");
        }
    }
    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
	httpd_resp_send(req, page, strlen(page)); 
     
    return ESP_OK;    
}

static void pzem_register_http_handler(httpd_handle_t _server)
{
    add_uri_get_handler( _server, PZEM_URI, pzem_get_handler, NULL); 
}

void pzem_http_init(httpd_handle_t _server)
{
    
    pzem_register_http_print_data();    
    pzem_register_http_handler(_server);
}


#endif