#include "ledcontrol_http.h"
#include "http_page_tpl.h"
#include "ledcontrol_mqtt.h"

#ifdef CONFIG_LED_CONTROL_HTTP

static const char *ledc_title ICACHE_RODATA_ATTR = "Led channels Config";
const char *ledc_data ICACHE_RODATA_ATTR = "ledc_data%d";

const char *html_page_config_ledc_count ICACHE_RODATA_ATTR = "Channels count";
const char *html_page_config_ledc_channel ICACHE_RODATA_ATTR = "Channel %d (GPIO)";
const char *html_page_config_ledc_channel_gpio_name ICACHE_RODATA_ATTR = "g_ch%d";
const char *html_page_config_ledc_channel_title_name ICACHE_RODATA_ATTR = "t_ch%d";
const char *html_page_config_ledc_count_name ICACHE_RODATA_ATTR = "cnt";

const char *html_block_led_control_item ICACHE_RODATA_ATTR = 
    "<p>"
        "<span class='lf'><b>%s</b></span>"                 // s - title,
        //"<span><input type=\"range\" max=\"255\" name=\"ledc%d\" value=\"%d\"></span>"          // slider, d - channel id, d - duty
        "<span class='rh'>"
            "<input type=\"range\" max=\"255\" name=\""LED_CONTROL_TAG"%d\" value=\"%d\" data-uri=\""LED_CONTROL_TAG"?ch=%d&duty=\" onchange=\"slider(this.value, this.name, this.dataset.uri);\" />"
            "<i id=\""LED_CONTROL_TAG"%d\" >%d</i>"
        "</span>"              // d - channel id, d - duty
    "</p>"; 

static const char* TAG = "LEDCHTTP";

#define LED_CONTROLLER_GROUP_TITLE_DEFAULT "LED Controller"

ledcontrol_group_t *led_groups;
uint8_t led_groups_count = 0;

static void ledc_print_options(http_args_t *args)
{
    ESP_LOGI( TAG, LOG_FMT());

    httpd_req_t *req = (httpd_req_t *)args->req;
    ledcontrol_t *ledc = (ledcontrol_t *)args->dev;


    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, ledc_title);
    // ==========================================================================    
    // print form with options
    httpd_resp_sendstr_chunk(req, html_block_data_form_start);
    
    
    // кол-во каналов
    char value[4];
    itoa(ledc->led_cnt, value, 10);
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                    , html_page_config_ledc_count // %s label
                                    , html_page_config_ledc_count_name   // %s name
                                    , value   // %d value
    );
        
    for ( uint8_t i = 0; i < ledc->led_cnt; i++)
    {
        char label[ strlen(html_page_config_ledc_channel)+1];
        char param[4];
        sprintf(label, html_page_config_ledc_channel, i);
        sprintf(param, html_page_config_ledc_channel_gpio_name, i);
        itoa(ledc->channels[i].pin, value, 10);
	    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                        , label // %s label
                                        , param   // %s name
                                        , value   // %d value
        );
    }

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_submit, LED_CONTROL_TAG);
    httpd_resp_sendstr_chunk(req, html_block_data_form_end);
    // ==========================================================================    
    httpd_resp_sendstr_chunk(req, html_block_data_end);    
    httpd_resp_sendstr_chunk(req, html_block_data_end);    
}

static void ledc_http_process_params(httpd_req_t *req, void *args)
{
    ESP_LOGI( TAG, LOG_FMT());
    
    ledcontrol_t *ledc = (ledcontrol_t *)((http_args_t *)args)->dev;

	if ( http_get_has_params(req) == ESP_OK) 
	{
        char param[50];
        if ( http_get_key_str(req, "st", param, sizeof(param)) == ESP_OK ) {
            // сверим что пришло в st
            if ( strcmp(param, LED_CONTROL_TAG) == 0 ) {
                // продолжим обработку параметров    

                // кол-во каналов  html_page_config_ledc_count_name
                if ( http_get_key_str(req, html_page_config_ledc_count_name, param, sizeof(param)) == ESP_OK ) {
                    // есть параметр
                    uint8_t cnt = atoi(param);
                    ESP_LOGI(TAG, LOG_FMT("received channels count %d"), cnt);

                    ledcontrol_nvs_data_t *data = NULL;
                    if ( cnt > 0 ) data = calloc(cnt, sizeof(ledcontrol_nvs_data_t));

                    char param[10];
                    uint8_t value;
                    for ( uint8_t i = 0; i < cnt; i++)
                    {
                        data[i].channel = i;
                        sprintf(param, html_page_config_ledc_channel_gpio_name, i);
                        if ( http_get_key_uint8(req, param, &data[i].pin) != ESP_OK ) 
                        {
                            ESP_LOGE(TAG, "gpio for channel %d is missing", i);
                            continue;
                        }

                        ESP_LOGI(TAG, LOG_FMT("received gpio %d for channel %d"), data[i].pin, data[i].channel);

                        /* 
                        TODO: "inverted" param
                         TODO: "title" param
                        */ 
                    }

                    ledcontrol_save_nvs(cnt, data);
                    // TODO: reinitialize ledcontroller
                    // ledcontrol_destroy( ledc );
                    // ledcontrol_create()
                    // ledc->register_channel
                    // или ledc->led_cnt =
                    // ledc->channels = realloc(...)
                    free(data);
                    
                    //ledcontrol_channel_t *ledc_channels;
                    
                    // It works!!!! but...  memory leaks ( 80 -200 bytes), каждое пересохранение
                    uint8_t led_ch_cnt = ledcontrol_init_channels(&ledc->channels);
                    ledcontrol_reinit(ledc, led_ch_cnt, &ledc->channels);
                    ledcontrol_mqtt_init(ledc);
                    //free(ledc_channels);
                }
                // else {} / 
            }
            //else { }
        }
    }    
}

static void ledcontrol_add_initial_group(ledcontrol_handle_t dev_h)
{
    if ( dev_h == NULL ) return;
    if (  led_groups_count == 0 )
    {
        led_groups_count++;
        led_groups = (ledcontrol_group_t *) realloc( led_groups, sizeof(ledcontrol_group_t) * led_groups_count );

        led_groups[ led_groups_count - 1 ].title = LED_CONTROLLER_GROUP_TITLE_DEFAULT;
        led_groups[ led_groups_count - 1 ].group = 0;
        led_groups[ led_groups_count - 1 ].dev_h = dev_h;
        led_groups[ led_groups_count - 1 ].priority = 5;
    }
}

static void ledcontrol_print_data(http_args_t *args)
{
    http_args_t *arg = (http_args_t *)args;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    ledcontrol_group_t *group = (ledcontrol_group_t *)arg->dev;
    ledcontrol_handle_t ledc_h = group->dev_h;
    ledcontrol_t *ledc = (ledcontrol_t *)ledc_h;
    
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, group->title);

    for (uint8_t i = 0; i < ledc->led_cnt; i++ ) 
    {   
        ledcontrol_channel_t *ch = ledc->channels + i;
        if ( group->group == ch->group)
        {        
            httpd_resp_sendstr_chunk_fmt(req, html_block_led_control_item
                                            , ch->name
                                            , ch->channel     // channel num
                                            , ch->duty        // channel duty    
                                            , ch->channel     // for data-uri                                  
                                            , ch->channel     // channel num
                                            , ch->duty        // channel duty
                                        );
        }
    }
    
    httpd_resp_sendstr_chunk(req, html_block_data_end);
}

void ledcontrol_register_http_print_data(ledcontrol_handle_t dev_h)
{
    if ( dev_h == NULL ) return;
    ledcontrol_t *ledc = (ledcontrol_t *)dev_h;

    // регистрация отображения блоков групп каналов на главной
    for ( uint8_t i = 0; i < led_groups_count; i++ )
    {
        // проверить, что группа не пустая
        uint8_t found = 0;
        for (uint8_t k = 0; k < ledc->led_cnt; k++ ) 
        {
            ledcontrol_channel_t *ch = ledc->channels + k;
            found = ( ch->group == led_groups[i].group);
            if ( found ) break;
        }

        // не выводим блок для пустой группы
        if ( found )
        {
            char block_name[20];
            snprintf(block_name, 20, ledc_data, i);
            http_args_t *p = calloc(1,sizeof(http_args_t));
            p->dev = &led_groups[i];
            register_print_page_block( block_name, PAGES_URI[ PAGE_URI_ROOT], led_groups[i].priority, ledcontrol_print_data, p, NULL, NULL );
        }
    }
    
    // регистрация блока настроек в /tools
    http_args_t *p = calloc(1,sizeof(http_args_t));
    p->dev = dev_h;
    register_print_page_block( "ledc_opt", PAGES_URI[ PAGE_URI_TOOLS], 3, ledc_print_options, p, ledc_http_process_params, p );
}

esp_err_t ledcontrol_get_handler(httpd_req_t *req)
{
    /*
    ip/ledc?ch=<channel>&duty=<duty>
    ip/ledc?ch=<channel>&on=1
    ip/ledc?ch=<channel>&off=1
    ip/ledc?ch=<channel>&step=<step>   // next duty
    ip/ledc?ch=<channel>&fade=1&from=<duty_from>&to=<duty_to>&delay=<duty_delay>
    ip/ledc?allon=1
    ip/ledc?alloff=1
    ip/ledc?ch=<channel> - print channel duty
    ip/ledc?all=1 - print all channels duty
*/
    esp_err_t err = ESP_FAIL;
    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
    // check params
	if ( http_get_has_params(req) == ESP_OK) 
	{
        user_ctx_t *ctx = req->user_ctx;
        ledcontrol_handle_t ledc_h = (ledcontrol_handle_t)ctx->args;
        ledcontrol_t *ledc = (ledcontrol_t *)ledc_h;


        char param[100];
        if ( http_get_key_str(req, "ch", param, sizeof(param)) == ESP_OK ) 
        {
            err = ESP_OK;
            uint8_t ch = atoi(param);
            if ( ch < 0 || ch > ledc->led_cnt /*LEDCONTROL_CHANNEL_MAX*/ ) 
            {
                err = ESP_FAIL;
                httpd_resp_sendstr_chunk(req, html_error);
                goto end;
            }    

            ledcontrol_channel_t *channel = ledc->channels + ch;
            long val = 0;

            if ( http_get_key_long(req, "duty", &val) == ESP_OK ) 
            {
                // channel->set_duty
                if ( val >= 0 && val <= MAX_DUTY ) 
                {
                    err = ledc->set_duty( channel, val );
                    ledc->update();
                }
            } 
            else if ( http_get_key_long(req, "on", &val) == ESP_OK ) 
            {
                // channel > on
                if ( val == 1) 
                    err = ledc->on(channel);
            } 
            else if ( http_get_key_long(req, "off", &val) == ESP_OK ) 
            {
                // channel > off
                if ( val == 1) 
                    err = ledc->off(channel);
            } 
            else if ( http_get_key_long(req, "step", &val) == ESP_OK ) 
            {
                if ( val > 0 ) 
                    err = ledc->next_duty(channel, val);    
                else if ( val < 0 ) 
                    err = ledc->prev_duty(channel, val*(-1));   
            } 
            else if ( http_get_key_long(req, "fade", &val) == ESP_OK ) 
            {
                // channel > fade
                long from, to, delay;
                if ( http_get_key_long(req, "from", &from) == ESP_OK &&
                     http_get_key_long(req, "to", &to) == ESP_OK &&
                     http_get_key_long(req, "delay", &delay) == ESP_OK )
                {
                    err = ledc->fade( channel, from, to, delay);
                }
            }

            if ( err == ESP_OK )
            {
                char *buf = malloc(3);
                itoa( ledc->get_duty( channel), buf, 10);
                httpd_resp_sendstr_chunk(req, buf);
                free(buf);
            }
            else
                httpd_resp_sendstr_chunk(req, html_error);
        } 
        else if ( http_get_key_str(req, "allon", param, sizeof(param)) == ESP_OK ) 
        {
            ledc->on_all();
            httpd_resp_sendstr_chunk(req, "OK");
        } 
        else if ( http_get_key_str(req, "alloff", param, sizeof(param)) == ESP_OK ) 
        {
            ledc->off_all();
            httpd_resp_sendstr_chunk(req, "OK");
        }            
        else if ( http_get_key_str(req, "all", param, sizeof(param)) == ESP_OK )
        {
            // print all duties and channels
            httpd_resp_sendstr_chunk(req, "{");
            for (uint8_t i = 0; i < ledc->led_cnt; i++ ) {             
                char s[12];
                
                ledcontrol_channel_t *ch = ledc->channels + i;
                uint8_t val = ledc->get_duty( ch );                
                sprintf(s, "\"ch%d\": %d", i, val);
                httpd_resp_sendstr_chunk(req, s);  
                if ( i < ledc->led_cnt-1 ) 
                    httpd_resp_sendstr_chunk(req, ", ");      
            }
            httpd_resp_sendstr_chunk(req, "}");            
        }
        
    }
    else
    {
        // no params, just show config page

    }
end:
	//httpd_resp_send_chunk(req, NULL, 0);
    httpd_resp_end(req);
    return ESP_OK;    
}

void ledcontrol_register_http_handler(httpd_handle_t _server, ledcontrol_handle_t dev_h)
{
    if ( dev_h == NULL ) return;
    user_ctx_t *ctx = (user_ctx_t *)calloc(1, sizeof(user_ctx_t));
    ctx->args = dev_h;
    add_uri_get_handler( _server, LED_CONTROL_URI, ledcontrol_get_handler, ctx); 
}

void ledcontrol_http_init(httpd_handle_t _server, ledcontrol_handle_t dev_h)
{
    if (dev_h == NULL) return;
    ledcontrol_add_initial_group(dev_h);
    ledcontrol_register_http_print_data(dev_h);  
    ledcontrol_register_http_handler(_server, dev_h);
}

void ledcontrol_http_add_group(ledcontrol_handle_t dev_h, const char *title, uint8_t num, uint8_t priority)
{
    if ( dev_h == NULL ) return;
    ledcontrol_add_initial_group(dev_h);

    uint8_t i = 0;
    for ( i = 0; i < led_groups_count; i++ ) 
    {
        if ( strcmp(led_groups[i].title, title) == 0 && led_groups[i].dev_h == dev_h)
        {
            // found, change name and priority
            break;
        }    
    }
    
    if ( i == led_groups_count )
    {
        led_groups_count++;
        led_groups = (ledcontrol_group_t *) realloc( led_groups, sizeof(ledcontrol_group_t) * led_groups_count );
    }
    led_groups[ i ].title = title;
    led_groups[ i ].group = num;
    led_groups[ i ].dev_h = dev_h;
    led_groups[ i ].priority = priority;

}

#endif