#include "http_utils.h"

static const char *TAG = "http_utils";

esp_err_t http_get_has_params(httpd_req_t *req){
    return ( httpd_req_get_url_query_len(req) > 0 ) ? ESP_OK : ESP_FAIL;
}


esp_err_t http_get_key_str(httpd_req_t *req, const char *param_name, char *value, size_t size){
    // get params 
    esp_err_t error = ESP_FAIL;
    char*  buf;
    size_t buf_len;
    
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len <= 1) return error;
    
    buf = malloc(buf_len);
    error = httpd_req_get_url_query_str(req, buf, buf_len);
    if ( error == ESP_OK) {
        ESP_LOGD(TAG, "Found URL query => %s", buf);
        /* Get value of expected key from query string */
        error = httpd_query_key_value(buf, param_name, value, size);
        if ( error == ESP_OK) 
                ESP_LOGD(TAG, "Found URL query parameter => %s=%s", param_name, value);
        else ESP_LOGD(TAG, esp_err_to_name( error ));
    }       
    free(buf);
    return error;
}

esp_err_t http_get_key_long(httpd_req_t *req, const char *param_name, long *value){
    ESP_LOGD(TAG, __func__);
    // get params 
    char*  buf;
    size_t buf_len;
    uint8_t error = 1;
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGD(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, param_name, param, sizeof(param)) == ESP_OK) {
                ESP_LOGD(TAG, "Found URL query parameter => %s=%s", param_name, param);
                error = str_to_int((int *)value, param, 10);
                ESP_LOGD(TAG, "%s = %li", param_name, *value);
            }             
        }        
        free(buf);
    }
    return (error == 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t http_get_key_uint16(httpd_req_t *req, const char *param_name, uint16_t *value){
    ESP_LOGD(TAG, __func__);
    // get params 
    char*  buf;
    size_t buf_len;
    uint8_t error = 1;
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGD(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, param_name, param, sizeof(param)) == ESP_OK) {
                ESP_LOGD(TAG, "Found URL query parameter => %s=%s", param_name, param);
                error = str_to_uint16(value, param, 10);
                ESP_LOGD(TAG, "%s = %d", param_name, *value);
            }              
        }        
        free(buf);
    }
    return (error == 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t http_get_key_uint16_def(httpd_req_t *req, const char *param_name, uint16_t *value, uint16_t def)
{
    esp_err_t err = http_get_key_uint16(req, param_name, value);
    if ( err != ESP_OK ) 
    {
        *value = def;
        ESP_LOGE(TAG, "error get param \"%s\", set to default %d", param_name, def);
    } else {
        ESP_LOGI(TAG, "get param \"%s\" = %d", param_name, *value);
    }
    return err;
}

esp_err_t http_get_key_uint8(httpd_req_t *req, const char *param_name, uint8_t *value){
    ESP_LOGD(TAG, __func__);
    // get params 
    char*  buf;
    size_t buf_len;
    uint8_t error = 1;
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGD(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, param_name, param, sizeof(param)) == ESP_OK) {
                ESP_LOGD(TAG, "Found URL query parameter => %s=%s", param_name, param);
                error = str_to_uint8(value, param, 10);
                ESP_LOGD(TAG, "%s = %d", param_name, *value);
            }              
        }        
        free(buf);
    }
    return (error == 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t http_get_key_uint8_def(httpd_req_t *req, const char *param_name, uint8_t *value, uint8_t def)
{
    esp_err_t err = http_get_key_uint8(req, param_name, value);
    if ( err != ESP_OK ) 
    {
        *value = def;
        ESP_LOGE(TAG, "error get param \"%s\", set to default %d", param_name, def);
    } else {
        ESP_LOGI(TAG, "get param \"%s\" = %d", param_name, *value);
    }
    return err;
}

char *http_uri_clean(httpd_req_t *req)
{
    char *p;
    if ( http_get_has_params(req) == ESP_OK) 
	{
        p = copy_str_from_str( req->uri, "?");
    } else {
        p = (char *) calloc(1, strlen( req->uri));
        strcpy(p, req->uri);
    }
    return p;  
}