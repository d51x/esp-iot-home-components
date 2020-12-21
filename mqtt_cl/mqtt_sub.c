#include "mqtt_sub.h"
#include "string.h"

#ifdef CONFIG_SENSOR_MQTT
static const char *TAG = "MQTTSUBS";

const char *html_page_title_mqtt_sensors ICACHE_RODATA_ATTR = "MQTT Sensors";
const char *html_page_title_mqtt_sensors_cfg ICACHE_RODATA_ATTR = "MQTT Sensors Config";

static uint8_t base_topics_count = 0;
static uint8_t end_points_count = 0;

static mqtt_sub_endpoint_t *end_points = NULL;
static mqtt_sub_base_topic_t *base_topics = NULL;
static mqtt_sub_endpoint_value_t *endpoint_values = NULL;

#define MQTT_SUBS_NVS_SECTION "mqttsubs"
#define MQTT_SUBS_NVS_KEY_BASE_COUNT "basecnt"
#define MQTT_SUBS_NVS_KEY_BASE_DATA  "basedata"

#define MQTT_SUBS_NVS_KEY_ENDPOINT_COUNT "endpntcnt"
#define MQTT_SUBS_NVS_KEY_ENDPOINT_DATA  "endpntdata"


static void debug_print_endpoints()
{
    ESP_LOGI(TAG, "base topic count %d", base_topics_count);
    ESP_LOGI(TAG, "end points count %d", end_points_count);

    if ( base_topics_count > 0 )
    {
        for ( uint8_t i = 0; i < base_topics_count; i++)
        {
            ESP_LOGI(TAG, "base topic %d: %s", i, base_topics[i].base);

            if ( end_points_count > 0 )
            {
                for ( uint8_t k = 0; k < end_points_count; k++)
                {
                    if ( end_points[k].base_id == i ) 
                    {
                        ESP_LOGI(TAG, "\t\t endpoint (%02d): %s", i, end_points[k].endpoint);
                    }
                }
            }
        }
    }    
}

char *mqtt_subscriber_get_full_topic_by_endpoint_id(uint8_t id)
{
    char *t = calloc(1, MQTT_SUBSCRIBER_BASE_TOPIC_MAX_LENGTH + MQTT_SUBSCRIBER_END_POINT_MAX_LENGTH + 1 ); 
    strcpy(t, base_topics[ end_points[id].base_id ].base);
    strcat(t + strlen(t), "/");
    strcat(t + strlen(t), end_points[id].endpoint);
    return t;
}

void mqtt_subscriber_clear_all()
{
    nvs_param_erase(MQTT_SUBS_NVS_SECTION, MQTT_SUBS_NVS_KEY_BASE_COUNT);
    nvs_param_erase(MQTT_SUBS_NVS_SECTION, MQTT_SUBS_NVS_KEY_BASE_DATA);
    nvs_param_erase(MQTT_SUBS_NVS_SECTION, MQTT_SUBS_NVS_KEY_ENDPOINT_COUNT);
    nvs_param_erase(MQTT_SUBS_NVS_SECTION, MQTT_SUBS_NVS_KEY_ENDPOINT_DATA);

    base_topics_count = 0;
    end_points_count = 0;

    if ( base_topics ) free( base_topics );
    if ( end_points ) free( end_points );
    if ( endpoint_values ) free( endpoint_values );
}

static void mqtt_subscriber_load_nvs()
{
    esp_err_t err = nvs_param_u8_load(MQTT_SUBS_NVS_SECTION, MQTT_SUBS_NVS_KEY_BASE_COUNT, &base_topics_count);
    if ( err != ESP_OK ) {
        ESP_LOGE(TAG, "%s: %s %s", __func__, esp_err_to_name(err), MQTT_SUBS_NVS_KEY_BASE_COUNT);
        base_topics_count = 0;
        end_points_count = 0;
        return;
    }    

    base_topics = (mqtt_sub_base_topic_t *) calloc(base_topics_count, sizeof(mqtt_sub_base_topic_t));
    err = nvs_param_load(MQTT_SUBS_NVS_SECTION, MQTT_SUBS_NVS_KEY_BASE_DATA, base_topics);

    if ( err != ESP_OK ) {
        ESP_LOGE(TAG, "%s: %s %s", __func__, esp_err_to_name(err), MQTT_SUBS_NVS_KEY_BASE_DATA);
        memset(base_topics, 0, base_topics_count * sizeof(mqtt_sub_base_topic_t));
        return;
    }     

    err = nvs_param_u8_load(MQTT_SUBS_NVS_SECTION, MQTT_SUBS_NVS_KEY_ENDPOINT_COUNT, &end_points_count);
    if ( err != ESP_OK ) {
        ESP_LOGE(TAG, "%s: %s %s", __func__, esp_err_to_name(err), MQTT_SUBS_NVS_KEY_ENDPOINT_COUNT);
        end_points_count = 0;
        return;
    } 

    end_points = (mqtt_sub_endpoint_t *) calloc(end_points_count, sizeof(mqtt_sub_endpoint_t));
    endpoint_values = (mqtt_sub_endpoint_value_t *) calloc(end_points_count, sizeof(mqtt_sub_endpoint_value_t));
    memset(endpoint_values, 0, end_points_count * sizeof(mqtt_sub_endpoint_value_t));
    err = nvs_param_load(MQTT_SUBS_NVS_SECTION, MQTT_SUBS_NVS_KEY_ENDPOINT_DATA, end_points);
    if ( err != ESP_OK ) {
        ESP_LOGE(TAG, "%s: %s %s", __func__, esp_err_to_name(err), MQTT_SUBS_NVS_KEY_ENDPOINT_DATA);
        memset(end_points, 0, end_points_count * sizeof(mqtt_sub_endpoint_t));
        return;
    }    
}

static void mqtt_subscriber_save_nvs_base_topics()
{
    esp_err_t err = nvs_param_u8_save(MQTT_SUBS_NVS_SECTION, MQTT_SUBS_NVS_KEY_BASE_COUNT, base_topics_count);
    if ( err != ESP_OK ) {
        ESP_LOGE(TAG, "%s: %s", __func__, esp_err_to_name(err));
    }

    err = nvs_param_save(MQTT_SUBS_NVS_SECTION, MQTT_SUBS_NVS_KEY_BASE_DATA, base_topics, base_topics_count*sizeof(mqtt_sub_base_topic_t));
    if ( err != ESP_OK ) {
        ESP_LOGE(TAG, "%s: %s", __func__, esp_err_to_name(err));
    }
}

static void mqtt_subscriber_save_nvs_end_points()
{
    esp_err_t err = nvs_param_u8_save(MQTT_SUBS_NVS_SECTION, MQTT_SUBS_NVS_KEY_ENDPOINT_COUNT, end_points_count);
    if ( err != ESP_OK ) {
        ESP_LOGE(TAG, "%s: %s", __func__, esp_err_to_name(err));
    }

    err = nvs_param_save(MQTT_SUBS_NVS_SECTION, MQTT_SUBS_NVS_KEY_ENDPOINT_DATA, end_points, end_points_count*sizeof(mqtt_sub_endpoint_t));
    if ( err != ESP_OK ) {
        ESP_LOGE(TAG, "%s: %s", __func__, esp_err_to_name(err));
    }
}

static int mqtt_subscriber_get_endpoint_id(uint8_t base_id, char *_endpoint)
{
    for (uint8_t i = 0; i < end_points_count; i++)
    {
        if ( end_points[i].base_id == base_id && strcmp( end_points[i].endpoint, _endpoint) == 0) {
            return i;
        }
    }
    return -1;
}

static int mqtt_subscriber_get_endpoint_id_by_topic(const char *topic)
{
    int res = -1;
    for (uint8_t i = 0; i < end_points_count; i++)
    {
        char *t = mqtt_subscriber_get_full_topic_by_endpoint_id(i);

        if ( strcmp(topic, t) == 0) {
            res = i;  // нашли endpoint id
            free(t);
            break;
        }

        free(t);
    }
    return res;
}

static void mqtt_subscriber_receive_cb(char *buf, void *args)
{
    // в args положить topic
    char *topic = (char *)args;
    // ищем endpoint id
    
    int endpoint_id = mqtt_subscriber_get_endpoint_id_by_topic(topic);
    
    if ( endpoint_id > -1 )
    {
        strcpy(endpoint_values[ endpoint_id ].value, buf);
    }
}

static void mqtt_subscriber_del_endpoints(uint8_t base_id)
{
    int8_t _endpoints_count = 0;
    for (uint8_t i = 0; i < end_points_count; i++)
    {
       if ( end_points[i].base_id == base_id)  _endpoints_count++;
    }
 
    if ( _endpoints_count > 0) 
    {
        mqtt_sub_endpoint_t *_end_points = (mqtt_sub_endpoint_t *)calloc( end_points_count - _endpoints_count, sizeof(mqtt_sub_endpoint_t));
        mqtt_sub_endpoint_value_t *_end_point_values = (mqtt_sub_endpoint_value_t *)calloc( end_points_count - _endpoints_count, sizeof(mqtt_sub_endpoint_value_t));

        uint8_t k = 0;
        // копируем endpoints во временный массив, которые не принадлежат указанному base_id
        for (uint8_t i = 0; i < end_points_count; i++)
        {
            if ( end_points[i].base_id != base_id ) 
            {
                _end_points[k].base_id = end_points[i].base_id;
                _end_points[k].id = k;

                strcpy(_end_points[k].endpoint, end_points[i].endpoint);
                
                _end_point_values[k].id = k;
                strcpy(_end_point_values[k].value, endpoint_values[i].value);
                
                k++;
            } else {
                // а те, которые принадлежат указанному base_id, отпишемся от них
                char *t = mqtt_subscriber_get_full_topic_by_endpoint_id(i);                
                mqtt_unsubscribe_topic(t);
                free(t);
            }
        }

        end_points_count -= _endpoints_count; 
        end_points = (mqtt_sub_endpoint_t *)realloc( end_points, end_points_count * sizeof(mqtt_sub_endpoint_t)); 
        endpoint_values = (mqtt_sub_endpoint_value_t *)realloc( endpoint_values, end_points_count * sizeof(mqtt_sub_endpoint_value_t)); 

        for (uint8_t i = 0; i < end_points_count; i++)
        {
            end_points[i].base_id = _end_points[i].base_id;
            end_points[i].id = i;
            strcpy(end_points[i].endpoint, _end_points[i].endpoint);
            endpoint_values[i].id = i;
            strcpy(endpoint_values[i].value, _end_point_values[i].value);
        }       

        free(_end_points);
        free(_end_point_values);
    }  
}

static void mqtt_subscriber_del_base_topic(uint8_t base_id)
{
    mqtt_sub_base_topic_t *_base_topics = (mqtt_sub_base_topic_t *) calloc( base_topics_count - 1, sizeof(mqtt_sub_base_topic_t) );
    uint8_t k = 0;
    for (uint8_t i = 0; i < base_topics_count; i++)
    {
        if ( base_topics[i].id != base_id )
        {
            _base_topics[k].id = base_topics[i].id;
            strcpy(_base_topics[k].base,base_topics[i].base );
            k++;
        }
    }

    base_topics_count -= 1;
    base_topics = (mqtt_sub_base_topic_t *) realloc( base_topics, base_topics_count * sizeof(mqtt_sub_base_topic_t) );

    for (uint8_t i = 0; i < base_topics_count; i++)
    {
        base_topics[i].id = _base_topics[i].id;
        strcpy(base_topics[i].base, _base_topics[i].base );
    }
    free( _base_topics );
}

// добавляет новый endpoint для указанного base topic id
// если endpoint уже есть для указанного base topic id, то ничего не делает
static esp_err_t mqtt_subscriber_add_endpoints(uint8_t base_id, char *_endpoints)
{
    esp_err_t err = ESP_OK;

    char *s = strdup(_endpoints);
    char *e = malloc(1); //cut_str_from_str(_endpoints, ";");
    
    // удалить все endpoints для base_id
    mqtt_subscriber_del_endpoints(base_id);

    while ( e != NULL )
    {
        e = cut_str_from_str(s, ";");
       if ( e == NULL ) break;
            // добавляем новый endpoint 
            if ( end_points_count < MQTT_SUBSCRIBER_MAX_END_POINTS ) 
            {
                end_points_count++;
                
                end_points = (mqtt_sub_endpoint_t *) realloc(end_points, end_points_count * sizeof(mqtt_sub_endpoint_t));
                endpoint_values = (mqtt_sub_endpoint_value_t *) realloc(endpoint_values, end_points_count * sizeof(mqtt_sub_endpoint_value_t));

                end_points[ end_points_count - 1 ].id = end_points_count - 1;
                end_points[ end_points_count - 1 ].base_id = base_id;
                strcpy(end_points[ end_points_count - 1 ].endpoint, e );
                endpoint_values[ end_points_count - 1 ].id = end_points_count - 1;
                strcpy(endpoint_values[ end_points_count - 1 ].value, "");
                
                char *t = mqtt_subscriber_get_full_topic_by_endpoint_id(end_points_count - 1); 
                mqtt_subscribe_topic(t);
                mqtt_add_receive_callback(t, 0, mqtt_subscriber_receive_cb, NULL);
                free(t);
            } else {
                ESP_LOGE(TAG, "Not slots (%d) available for new endpoint %s", MQTT_SUBSCRIBER_MAX_END_POINTS, e);
                err = ESP_FAIL;
                break;
            }
    }
    free(e);
    free(s);
    return err;
}

static int mqtt_subscriber_get_base_topic_id(const char *base)
{
    for ( uint8_t i = 0; i < base_topics_count; i++)
    {
        if ( strcmp(base, base_topics[i].base) == 0) {
            return i;  // нашли base topic
        }
    }

    // не нашли
    return -1;
}

// добавляет список endpoints в указанный base topic
// 
esp_err_t mqtt_subscriber_add(const char* base_topic, const char *_endpoints)
{
    int base_id = mqtt_subscriber_get_base_topic_id(base_topic);

    if ( base_id == -1 )
    {
        // можно добавлять новый топик
        if  ( base_topics_count < MQTT_SUBSCRIBER_MAX_BASE_TOPICS )
        { 
            base_topics_count++;
            base_id = base_topics_count-1;

            // увеличим массив
            base_topics = (mqtt_sub_base_topic_t *) realloc(base_topics, base_topics_count * sizeof(mqtt_sub_base_topic_t));
            base_topics[ base_id ].id = base_id;
            strcpy(base_topics[ base_id ].base, base_topic );
            mqtt_subscriber_save_nvs_base_topics();
        } else {
            ESP_LOGE(TAG, "Not slots (%d) available for new base topic %s", MQTT_SUBSCRIBER_MAX_BASE_TOPICS,  base_topic);
            return ESP_FAIL;           
        }
    } 

    // продолжаем, если base_id > -1
    esp_err_t err = mqtt_subscriber_add_endpoints( base_id, _endpoints);
     if ( err == ESP_OK )
         mqtt_subscriber_save_nvs_end_points();
    return err;
}


static void rebase_endpoint_ids()
{
    for ( uint8_t i = 0; i < base_topics_count; i++ )
    {
        for ( uint8_t k = 0; k < end_points_count; k++)
        {
            if ( end_points[k].base_id == base_topics[i].id )
                {
                    end_points[k].base_id = i; // ????? не будет ли пересечений
                }
        }
    }

}

static void rebase_base_topic_ids()
{
    for ( uint8_t i = 0; i < base_topics_count; i++ )
    {
        base_topics[i].id = i;
    }    
}


// удаляет base topic и все его endpoints
esp_err_t mqtt_subscriber_del(const char* base_topic)
{
    int base_id = mqtt_subscriber_get_base_topic_id(base_topic);

    if ( base_id == -1 )
    {
        ESP_LOGE(TAG, "Unable find base topic \"%s\"", base_topic);
        return ESP_FAIL;
    }

    // удалим все endpoints найденного basetopic
    mqtt_subscriber_del_endpoints(base_id);
    
    // теперь удалим сам base_topic
    mqtt_subscriber_del_base_topic(base_id);


    // переназначить base_id для endpoints и id для base
    rebase_endpoint_ids();
    rebase_base_topic_ids();

    mqtt_subscriber_save_nvs_base_topics();
    mqtt_subscriber_save_nvs_end_points();

    return ESP_OK;
}

static void mqtt_subscriber_print_data_block(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, html_page_title_mqtt_sensors);
    httpd_resp_sendstr_chunk(req, html_block_data_form_start);

    for ( uint8_t i = 0 ; i < end_points_count; i++ )
    {
        char *t = mqtt_subscriber_get_full_topic_by_endpoint_id(i);

        httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_w65_label
                                    , t // %s label
                                    , endpoint_values[i].value
                                    );

        free(t);
    }
    httpd_resp_sendstr_chunk(req, html_block_data_end);
}


static void mqtt_subscriber_print_options(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, html_page_title_mqtt_sensors_cfg);

    httpd_resp_sendstr_chunk_fmt(req, "<button class='button norm rh2' onclick='mqttsuba(\"%s\",%d,%d)'>New</button>"
                                      "<div>"
                                      "<p> <span id='basecnt'>Base topics: %d</span><span>/%d</span></p>"
                                      "<p> <span id='endpcnt'>Endpoints: %d</span><span>/%d</span></p>"
                                      "</div>"
                                      "<hr>"
                , "mqttpcs"
                , base_topics_count
                , end_points_count
                , base_topics_count
                , MQTT_SUBSCRIBER_MAX_BASE_TOPICS
                , end_points_count
                , MQTT_SUBSCRIBER_MAX_END_POINTS
                );

    // print options (edit text)
    httpd_resp_sendstr_chunk(req, "<div id='mqttpcs'>");

    for ( uint8_t i = 0 ; i < base_topics_count; i++ )
    {
        char label[16] = "";
        char name[10] = "";
        sprintf(label, "Base topic %d", i+1 );
        sprintf(name, "base%d", i );

        httpd_resp_sendstr_chunk_fmt(req, "<div id=\"basetop%d\">", i);
        httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                    , label // %s label
                                    , name 
                                    , base_topics[i].base
                                    );
        uint8_t count = 0;
        for (uint8_t k=0; k < end_points_count; k++)
        {
            if ( end_points[k].base_id == i) count++;
        }

        char *_endpoints = NULL;        
        if ( count > 0 )
        {
            _endpoints = malloc( count * (MQTT_SUBSCRIBER_END_POINT_MAX_LENGTH + 1) + 1);
            strcpy(_endpoints, "");

            for (uint8_t k=0; k < end_points_count; k++)
            {
                if ( end_points[k].base_id == i) 
                {
                    strcat(_endpoints, end_points[k].endpoint);
                    if (count > 1 ) strcat( _endpoints, ";");
                    count--;
                } 
            }
        }
        
        sprintf(name, "endpt%d", i);
        httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                     , "endpoints" // %s label
                                     , name
                                     , _endpoints
                                     );                                    
        free(_endpoints);

        httpd_resp_sendstr_chunk_fmt(req, "<p class='lf2'>"
                                            "<button class='button norm rht' onclick='mqttsubs(%d,0)'>Set</button>"
                                            "<button class='button norm rht' onclick='mqttsubs(%d,1)'>Del</button>"
                                            "</p>"
        , i
        , i
        ); 
        
        httpd_resp_sendstr_chunk(req, "</div>");
        if (i < base_topics_count-1 ) httpd_resp_sendstr_chunk(req, "<hr>");
    }

    httpd_resp_sendstr_chunk(req, "</div>");
    httpd_resp_sendstr_chunk(req, html_block_data_end);  
}

void mqtt_subscriber_register_http_print_data()
{
    http_args_t *p = calloc(1,sizeof(http_args_t));
    register_print_page_block( "mqtt_subs", PAGES_URI[ PAGE_URI_ROOT ], 7, mqtt_subscriber_print_data_block, p, NULL, NULL );
    register_print_page_block( "mqtt_subs_opt", MQTT_SUBSCRIBER_URI, 2, mqtt_subscriber_print_options, p, NULL, NULL); 
}


esp_err_t mqtt_subscriber_get_handler(httpd_req_t *req)
{
        // check params
    char page[512] = ""; 
	if ( http_get_has_params(req) == ESP_OK) 
	{
        // /mqttsub?base0=dacha%2Fbathroom&endpt0=dhtt1%3Bdhth1&st=mqbt0
        httpd_req_get_url_query_str(req, page, 512);

        char param[10];
        if ( http_get_key_str(req, "clear", param, sizeof(param)) == ESP_OK ) {
            if ( strcmp(param, "all") == 0) {
                mqtt_subscriber_clear_all();
            }
        } else {
            if ( http_get_key_str(req, "act", param, sizeof(param)) == ESP_OK ) {
                esp_err_t err = ESP_OK;
                char _base_topic[MQTT_SUBSCRIBER_BASE_TOPIC_MAX_LENGTH+1];
                err |= http_get_key_str(req, "base", _base_topic, MQTT_SUBSCRIBER_BASE_TOPIC_MAX_LENGTH);

                char _endpoints[MQTT_SUBSCRIBER_END_POINT_MAX_LENGTH+1];
                err |= http_get_key_str(req, "endpt", _endpoints, MQTT_SUBSCRIBER_END_POINT_MAX_LENGTH);
                
                if ( err == ESP_OK )
                {                
                    switch ( atoi(param)) {
                        case 0:
                                mqtt_subscriber_add(_base_topic, _endpoints);
                                break;
                        case 1:
                                mqtt_subscriber_del(_base_topic);
                                break;
                    }
                }
            }
        }
    } 

    show_http_page( req );

    
	//httpd_resp_send_chunk(req, NULL, 0); 
    httpd_resp_end(req);
    return ESP_OK;
}

esp_err_t mqtt_subscriber_post_handler(httpd_req_t *req, void *args)
{
    if ( req->content_len == 0 ) {
        httpd_resp_end(req);
        return ESP_FAIL;
    }

    bool error = false;
    char *buf = calloc(1, req->content_len+1);

    if ( httpd_req_recv(req, buf, req->content_len) > 0)
    {
        // /mqttsub?base=dacha/bathroom&endpt=dhtt1;dhth1&act=0
        char _base_id[3];
        char _base_topic[MQTT_SUBSCRIBER_BASE_TOPIC_MAX_LENGTH+1];
        char _endpoints[MQTT_SUBSCRIBER_END_POINT_MAX_LENGTH+1];
        char _action[4];

        esp_err_t res;
        res = httpd_query_key_value(buf, "id", _base_id, 3);
        error |= res;

        if ( res != ESP_OK) 
            ESP_LOGE(TAG, esp_err_to_name( error ));  

        res = httpd_query_key_value(buf, "base", _base_topic, MQTT_SUBSCRIBER_BASE_TOPIC_MAX_LENGTH);
        error |= res;
        
        if ( error == ESP_OK ) {
            uint8_t action = atoi(_action);
            if ( action == 0 ) {
                // добавляем
                res = mqtt_subscriber_add(_base_topic, _endpoints); 
                error |= res;
            } else if ( action == 1 ) {
                // удаляем
                res = mqtt_subscriber_del(_base_topic); 
                error |= res;
            }
        }       
    }
    
    httpd_resp_set_status(req, HTTPD_TYPE_JSON);
    if ( error == ESP_OK ) 
    {
        strcpy(buf, "{\"error\":0}");
        
    } else {
        strcpy(buf, "{\"error\":1}");
    }
    
    httpd_resp_sendstr_chunk(req, buf);
    free(buf);
    httpd_resp_end(req);

    return ESP_OK;
}

void mqtt_subscriber_register_http_handler(httpd_handle_t _server)
{
    user_ctx_t *ctx = (user_ctx_t *)calloc(1, sizeof(user_ctx_t));
    strncpy(ctx->title, "Mqtt subscriber", 20);
    ctx->show = true; 

    add_uri_get_handler( _server, MQTT_SUBSCRIBER_URI, mqtt_subscriber_get_handler, ctx); 
    add_uri_post_handler(_server, MQTT_SUBSCRIBER_URI, mqtt_subscriber_post_handler, NULL);
}

void mqtt_subscriber_init(httpd_handle_t _server)
{
    //mqtt_subscriber_clear_all();
    mqtt_subscriber_load_nvs();
    //debug_print_endpoints();

    for (uint8_t i = 0; i < end_points_count; i++)
    {
        char *t = mqtt_subscriber_get_full_topic_by_endpoint_id(i);
        mqtt_add_receive_callback( t, 0, mqtt_subscriber_receive_cb, NULL);
        free(t);
    }
        
    mqtt_subscriber_register_http_print_data();
    mqtt_subscriber_register_http_handler(_server);
    register_http_page_menu( MQTT_SUBSCRIBER_URI, "MqttSub");
}
#endif