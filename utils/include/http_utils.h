#ifndef __HTTP_UTILS_H__
#define __HTTP_UTILS_H__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <limits.h>
#include <errno.h>
#include "sys/time.h"
//#include <ctype.h>

#include "esp_log.h"
#include "esp_system.h"
#include "rom/ets_sys.h"
#include "esp_http_server.h"
#include "esp_http_client.h"
#include <sys/param.h>
#include "utils.h"

typedef esp_err_t (*httpd_uri_func)(httpd_req_t *req);




esp_err_t http_get_has_params(httpd_req_t *req);
esp_err_t http_get_key_str(httpd_req_t *req, const char *param_name, char *value, size_t size);
esp_err_t http_get_key_long(httpd_req_t *req, const char *param_name, long *value);
esp_err_t http_get_key_uint16(httpd_req_t *req, const char *param_name, uint16_t *value);
esp_err_t http_get_key_uint8(httpd_req_t *req, const char *param_name, uint8_t *value);

char *http_uri_clean(httpd_req_t *req);
#endif