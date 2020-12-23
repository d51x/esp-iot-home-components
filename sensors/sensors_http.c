

#include "sensors_http.h"
#include "http_page_tpl.h"


static const char *TAG = "SENSHTTP";

const char *html_block_sensors_title ICACHE_RODATA_ATTR = "Сенсоры";

static void sensors_print_data(http_args_t *args)
{
    // http_args_t *arg = (http_args_t *)args;
    // httpd_req_t *req = (httpd_req_t *)arg->req;

}

static void sensors_register_http_print_data() 
{
    // http_args_t *p = calloc(1,sizeof(http_args_t));
    // register_print_page_block( "sensors_main", PAGES_URI[ PAGE_URI_ROOT], 3, sensors_print_data, p, NULL, NULL);
    
}

static esp_err_t sensors_get_handler(httpd_req_t *req)
{
    bool vsens = false;
    char param[2];
    vsens = ( http_get_key_str(req, "m", param, sizeof(param)) == ESP_OK );
    if ( !vsens )
    {
        httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
    }   

    // print sensor data
    size_t sz = get_buf_size("hostname:%s;", wifi_cfg->hostname);
    char *data = malloc(sz+1);
    sprintf(data, "hostname:%s;", wifi_cfg->hostname);
    
    for ( int16_t i = 0; i < sensors_count; i++)
    {
        // name:value;
        // sensors[i].name
        // sensors[i].fmt
        // sensors[i].val_type
        // sensors[i].value        
        // char sval[10];
        // switch ( sensors[i].val_type)
        // {
        //     case TYPE_UINT8:
        //     case TYPE_INT8:
        //     case TYPE_UINT16:
        //     case TYPE_INT16:
        //     case TYPE_UINT32:
        //     case TYPE_INT32:
        //         sprintf(sval, sensors[i].fmt, sensors[i].value);
        //         break;
        //     case TYPE_FLOAT:
        //         sprintf(sval, sensors[i].fmt, sensors[i].value.f);
        //         break;            
        //     case TYPE_STRING:
        //         break;
        // }

        // sz = get_buf_size("%s:%s;", sensors[i].name, sval);
        // data = (char *) realloc(data, strlen(data) + sz);
        // sprintf(data + strlen(data), "%s:%s;", sensors[i].name, sval);

        char *buf = calloc( 1, 1 );
        sensors[i].fn_cb(&buf, sensors[i].args) ;
        // print buf, buf must be formatted as "%s:%s;", name, val
        data = (char *) realloc(data, strlen(data) + strlen(buf));
        strcat(data, buf);
        free(buf);
    } 


    if ( vsens )
        httpd_send(req, data, strlen(data));
    else
        httpd_resp_send(req, data, strlen(data));

    free(data);  

    return ESP_OK;        
}

void sensors_http_init(httpd_handle_t _server)
{
    sensors_register_http_print_data();
    add_uri_get_handler( _server, SENSORS_URI, sensors_get_handler, NULL); 
}


