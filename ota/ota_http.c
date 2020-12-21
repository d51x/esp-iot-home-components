#include "ota_http.h"
#include "http_page_tpl.h"

static const char *TAG = "OTAHTTP";

static const char *ota_block_title ICACHE_RODATA_ATTR = "Firmware update"; 

// const char *ota_status_json ICACHE_RODATA_ATTR = "{\"state\": %d, \"progress\": %d, \"total\": %d}";

const char *html_page_failed ICACHE_RODATA_ATTR = "OTA upgrade failed...\n";
const char *html_page_uploaded ICACHE_RODATA_ATTR = "File uploaded, it took %d sec. Restarting....";

const char *html_page_ota ICACHE_RODATA_ATTR = 
  //"<p>Выбрать Firmware</p>"
  "<form enctype='multipart/form-data' method='post' action='"HTTP_URI_OTA"'>" 
    "<input type='file' name='file' id='selectedFile' style='display: none;'   accept='.bin'onchange='otaGet(%d)' />" 
    "<p><input type='button' class='button norm rh' value='Browse...' onclick=\"document.getElementById('selectedFile').click();\" /></p>"
    "<p><div class='lf2'><input type='submit' value='Загрузить' class='button norm rht' onclick=\"updFw()\" /></p>"
  //"</form>"
  ; 

const char *html_page_ota_info ICACHE_RODATA_ATTR = 
// "<p><b>Project name: </b>%s</p>"
// "<p><b>Installed version: </b>v.%s</p>"
// "<p><b>FileSize: </b> %d bytes</p>"
// "<p><b>Compiled: </b> %s %s</p>"
// "<p><b>Updated: </b> %s</p>"
// "<p><b>Filename: </b> %s</p>"
// "<p><b>Running partition: </b> %s (%d) bytes</p>"
// "<p><b>IDF Ver: </b> %s</p>"

"<table style='padding:0 10px;'>"
"<tr><td><b>Project name: </b></td><td>%s</td></tr>"
"<tr><td><b>Installed version: </b></td><td>v.%s</td></tr>"
"<tr><td><b>FileSize: </b></td><td>%d bytes</td></tr>"
"<tr><td><b>Compiled: </b></td><td> %s %s</td></tr>"
"<tr><td><b>Updated: </b></td><td> %s</td></tr>"
"<tr><td><b>Filename: </b></td><td> %s</td></tr>"
"<tr><td><b>Running partition: </b></td><td> %s (%d) bytes</td><tr>"
"<tr><td><b>IDF Ver: </b></td><td> %s</td></tr>"
"</table>";

const char *html_page_ota_selected ICACHE_RODATA_ATTR = "<h4 id=\"file_info\"></h4>"
"<h4 id=\"ota_process\"></h4>";

void ota_register_http_print_data();
void ota_http_process_params(httpd_req_t *req, void *args);
static void ota_debug_print(http_args_t *args);

static void ota_print_html(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, ota_block_title);

    //html_page_ota_info
    esp_app_desc_t *app_desc = esp_ota_get_app_description();
    const esp_partition_t* esp_part = esp_ota_get_running_partition();

    ota_firm_t *fw = malloc( sizeof(ota_firm_t));
    esp_err_t err = ota_load_nvs(fw);
    if ( err != ESP_OK ) {
        memset(fw, 0, sizeof(ota_firm_t));
    }

    httpd_resp_sendstr_chunk_fmt(req
    , html_page_ota_info
    , app_desc->project_name
    , app_desc->version
    , fw->size
    , app_desc->date
    , app_desc->time
    , fw->dt
    , fw->fname
    , esp_part->label
    , esp_part->size        
    , app_desc->idf_ver
    );
    
    free(fw);

    httpd_resp_sendstr_chunk_fmt(req, html_page_ota, esp_part->size);
    httpd_resp_sendstr_chunk(req, html_page_ota_selected);

    free(app_desc);

    httpd_resp_sendstr_chunk(req, html_block_data_end);  
    httpd_resp_sendstr_chunk(req, html_block_data_end);  
}

static void ota_debug_print(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;



    // httpd_resp_sendstr_chunk_fmt(req, 
    // "<br>Firmware (OTA): <br>"
    // "name: %s<br>"
    // "version: %s<br>"
    // "idf_ver: %s<br>"
    // "file: %s<br>"
    // "fw size: %d<br>"
    // "updated: %s<br>"
    // "compiled: %s %s<br>"
    // "running partition: %s (%d bytes)<br>"
    // , app_desc->project_name
    // , app_desc->version
    // , app_desc->idf_ver
    // , fw->fname
    // , fw->size
    // , fw->dt
    // , app_desc->date
    // , app_desc->time
    // , esp_part->label
    // , esp_part->size
    // );    


}

void ota_http_get_process_params(httpd_req_t *req, void *args)
{
    // TODO, ajax not responding while ota uploaded file, try websockets
    // /update?st=1
    // return upload status
	// if ( http_get_has_params(req) == ESP_OK) 
	// {
    //     char param[10];
    //     if ( http_get_key_str(req, "st", param, sizeof(param)) == ESP_OK ) {
    //         if ( atoi(param) != 1 ) {
    //             return;	
    //         }
    //         ESP_LOGW(TAG, ota_status_json, ota_status.state, ota_status.progress, ota_status.total);
    //         httpd_resp_set_type(req, HTTPD_TYPE_JSON);
    //         size_t sz = get_buf_size(ota_status_json, ota_status.state, ota_status.progress, ota_status.total);
    //         char *data = calloc(1, sz + 1);
    //         sprintf(data, ota_status_json, ota_status.state, ota_status.progress, ota_status.total);
    //         httpd_send(req, data, sz);
    //         free(data);
    //     }        
    // }    
}

void ota_http_post_process_params(httpd_req_t *req, void *args)
{
    char err_text[200];
    
    uint32_t start_time = millis(); 

    if ( ota_task_upgrade_from_web(req, err_text) == ESP_OK ) {
        ESP_LOGW(TAG, "Ota upgrade OK");
        // upgrading is OK, restart esp and redirect to main page in 10
        httpd_resp_set_hdr(req, "Refresh", "10; /");

        char *buf = malloc( strlen(html_page_uploaded) + 10);
        sprintf(buf, html_page_uploaded, (uint32_t)(millis()-start_time)/1000);
        httpd_resp_sendstr_chunk(req, buf);
        free(buf);
        

        ESP_LOGW("OTAHTTP", "Create Restart Task...");
        xTaskCreate(systemRebootTask, "sysreboot", 1024, 5000, 5, NULL); 
        ESP_LOGW("OTAHTTP", "Restart...");
    } else {
        ESP_LOGE(TAG, "OTA upgrade ERROR");
        
        httpd_resp_set_status(req, HTTPD_500);
        httpd_resp_sendstr_chunk(req, html_page_failed);
        httpd_resp_sendstr_chunk(req, err_text);
    }

    httpd_resp_end(req);
    return ESP_OK;
}

void ota_register_http_print_data() 
{
    http_args_t *p = calloc(1,sizeof(http_args_t));
    register_print_page_block( "ota", PAGES_URI[ PAGE_URI_OTA ], 1, ota_print_html, p, ota_http_get_process_params, NULL );
    //register_print_page_block( "debug", PAGES_URI[ PAGE_URI_DEBUG], 2, ota_debug_print, p, NULL, NULL  ); 

}

void ota_http_init(httpd_handle_t _server)
{
    ota_register_http_print_data();
    add_uri_post_handler(_server, HTTP_URI_OTAPOST, ota_http_post_process_params, NULL);
}