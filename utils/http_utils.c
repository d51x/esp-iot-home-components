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

esp_err_t http_get_key_long(httpd_req_t *req, const char *param_name, long *value, long def){
    char param[10];
    esp_err_t err = http_get_key_str(req, param_name, param, sizeof(param));
    
    if ( err == ESP_OK) 
    {
        err = ( strlen(param) > 0 ) ? ESP_OK : ESP_FAIL;
        *value = ( err == ESP_OK ) ? atol(param): def;
    }  
    else 
    {
        *value = def;
    }
    return err;
}

esp_err_t http_get_key_uint16(httpd_req_t *req, const char *param_name, uint16_t *value, uint16_t def)
{
    char param[10];
    esp_err_t err = http_get_key_str(req, param_name, param, sizeof(param));
    
    if ( err == ESP_OK) 
    {
        ESP_LOGD(TAG, "Found URL query parameter => %s=%s", param_name, param);
        err = ( strlen(param) > 0 ) ? ESP_OK : ESP_FAIL;
        *value = ( err == ESP_OK ) ? atoi(param): def;
        ESP_LOGD(TAG, "%s = %d", param_name, *value);
    }  
    else 
    {
        *value = def;
        ESP_LOGE(TAG, "error get param \"%s\", set to default %d", param_name, def);
    }
    return err;
}

esp_err_t http_get_key_uint8(httpd_req_t *req, const char *param_name, uint8_t *value, uint8_t def)
{
    char param[10];
    esp_err_t err = http_get_key_str(req, param_name, param, sizeof(param));
    
    if ( err == ESP_OK) 
    {
        ESP_LOGD(TAG, "Found URL query parameter => %s=%s", param_name, param);
        err = ( strlen(param) > 0 ) ? ESP_OK : ESP_FAIL;
        *value = ( err == ESP_OK ) ? atoi(param): def;
        ESP_LOGD(TAG, "%s = %d", param_name, *value);
    }  
    else 
    {
        *value = def;
        ESP_LOGE(TAG, "error get param \"%s\", set to default %d", param_name, def);
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