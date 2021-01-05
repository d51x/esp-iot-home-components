#include "i2c_http.h"
#include "http_page_tpl.h"


#ifdef CONFIG_COMPONENT_I2C

static const char *TAG = "I2C";

const char *URI_I2C_SCAN ICACHE_RODATA_ATTR = "/i2cscan";
static const char *OPTIONS_I2C ICACHE_RODATA_ATTR = "i2c_options";
static const char *TITLE_I2C_SDA ICACHE_RODATA_ATTR = "SDA";
static const char *TITLE_I2C_SCL ICACHE_RODATA_ATTR = "SCL";


static const char *I2C_SETTINGS__TITLE ICACHE_RODATA_ATTR = "I2C Settings";

#ifdef CONFIG_COMPONENT_I2C_SCANNER
const char *html_page_tools_i2c_scan ICACHE_RODATA_ATTR =                                   
                                    "<button id='i2cscan' class='button off rht' onclick='i2cscan()'>Сканировать</button>"
                                    "<div id='i2cres'></div>";   
#endif

void i2c_print_options(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    i2c_config_t *cfg = (i2c_config_t *) calloc(1, sizeof(i2c_config_t));
    i2c_load_cfg( cfg );

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, I2C_SETTINGS__TITLE);

    // ==========================================================================
    httpd_resp_sendstr_chunk(req, html_block_data_form_start);
    httpd_resp_sendstr_chunk(req, html_block_data_div_lf3);

    // ==========================================================================
    char param[2];
    itoa(cfg->sda_io_num , param, 10);
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                    , TITLE_I2C_SDA // %s label
                                    , PARAM_I2C_SDA   // %s name
                                    , param // %d value
                                );

    // ==========================================================================
    itoa(cfg->scl_io_num , param, 10);
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                    , TITLE_I2C_SCL // %s label
                                    , PARAM_I2C_SCL   // %s name
                                    , param  // %d value
                                );
    free(cfg);

    // ==========================================================================
    httpd_resp_sendstr_chunk(req, html_block_data_end);

    // ==========================================================================
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_submit, PARAM_I2C ); // %s st    

    // ==========================================================================
    //httpd_resp_sendstr_chunk(req, html_block_data_end);
    httpd_resp_sendstr_chunk(req, html_block_data_form_end);
#ifdef CONFIG_COMPONENT_I2C_SCANNER
    httpd_resp_sendstr_chunk(req, html_page_tools_i2c_scan);
#endif
    httpd_resp_sendstr_chunk(req, html_block_data_end);
}

/*
void i2c_register_http_menu()
{
    //register_http_page_menu( "/i2c", "I2C");
}
*/

void i2c_register_http_print_data() 
{
    http_args_t *p = calloc(1,sizeof(http_args_t));
    register_print_page_block( OPTIONS_I2C, PAGES_URI[ PAGE_URI_TOOLS], 3, i2c_print_options, p, i2c_http_process_params, NULL  );
}


void i2c_register_http_handler(httpd_handle_t _server)
{
    //ESP_LOGI(TAG, "function %s started", __func__);
    #ifdef CONFIG_COMPONENT_I2C_SCANNER
    add_uri_get_handler( _server, URI_I2C_SCAN, i2cscan_get_handler, NULL); 
    #endif    
}


void i2c_http_process_params(httpd_req_t *req, void *args)
{

   // check params
	if ( http_get_has_params(req) == ESP_OK) 
	{
        char param[100];
        if ( http_get_key_str(req, "st", param, sizeof(param)) == ESP_OK ) {
            if ( strcmp(param, PARAM_I2C) != 0 ) {
                return;	
            }
        } 
        
        i2c_config_t *cfg = (i2c_config_t *) calloc(1, sizeof(i2c_config_t));
        if ( http_get_key_str(req, PARAM_I2C_SDA, param, sizeof(param)) == ESP_OK ) {
            cfg->sda_io_num = atoi(param);
        }  else {
            cfg->sda_io_num = I2C_SDA_DEFAULT;
        }

        if ( http_get_key_str(req, PARAM_I2C_SCL, param, sizeof(param)) == ESP_OK ) {
            cfg->scl_io_num = atoi(param);
        }  else {
            cfg->scl_io_num = I2C_SCL_DEFAULT;
        }   

        i2c_save_cfg( cfg );
        free( cfg );
    } 
}

#ifdef CONFIG_COMPONENT_I2C_SCANNER
const char *i2c_devices_not_found ICACHE_RODATA_ATTR = "Устройства не найдены";

esp_err_t i2cscan_get_handler(httpd_req_t *req)
{
	uint8_t found = 0;	
    i2c_bus_handle_t bus_handle = i2c_bus_init();

    uint8_t sz = 15*20; 
    char *result = malloc( sz );
    strcpy(result, "");

    uint8_t *devices = malloc(128);
    uint8_t count = 0;
    count = i2c_bus_scan(bus_handle, devices);

    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
    if ( count == 0) {
        strcpy( result, i2c_devices_not_found );
    }

    for ( uint8_t i = 0; i<count;i++) {
        sprintf(result + strlen(result), "%d: 0x%02x<br>", i+1, devices[i]);
    }

    free(devices);
    devices = NULL;

	httpd_resp_send(req, result, -1);      
    free(result);
    return ESP_OK;
}

#endif

void i2c_http_init(httpd_handle_t _server)
{
    i2c_register_http_handler(_server);
    i2c_register_http_print_data();   
}

#endif //#ifdef CONFIG_COMPONENT_I2C