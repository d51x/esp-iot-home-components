#include "rgbcontrol_http.h"
#include "http_page_tpl.h"

#ifdef CONFIG_RGB_CONTROLLER_HTTP

static const char* TAG = "RGBHTTP";

extern const char *html_block_rgb_control_start ICACHE_RODATA_ATTR = 
    "<div class='group rnd'>"
        //"<h4 class='brd-btm'>Color Effects:</h4>";      
        "<h4 >Color Effects:</h4>";      

const char *html_block_rgb_control_end ICACHE_RODATA_ATTR = 
    "</div>";    

const char *color_box_data_start ICACHE_RODATA_ATTR = 
        "<div id='colors' style='background:rgb(%d,%d,%d)'></div>";

const char *effects_data_select_item ICACHE_RODATA_ATTR = "</div>";

 #ifdef CONFIG_RGB_EFFECTS
const char *effects_select_start ICACHE_RODATA_ATTR =  "<div class='ef'>"
                                                            "<select id=\"effects\" onchange=\"effects()\">";

    
const char *effects_select_end ICACHE_RODATA_ATTR = "</select></div>";

const char *effects_item ICACHE_RODATA_ATTR = "<option value=\"%d\" %s>%s</option>";
#endif



const char *effects_data_end ICACHE_RODATA_ATTR = "</div>";


static void rgbcontrol_print_color_sliders(httpd_req_t *req, char *data, rgbcontrol_t *rgb_ctrl)
{

    
    //sprintf(data+strlen(data), html_block_led_control_start, "RGB Controller");
    
    sprintf(data, html_block_data_header_start, "RGB Controller");
    httpd_resp_sendstr_chunk(req, data);

    // print color box
    color_rgb_t rgb;

    rgb.r = rgb_ctrl->ledc->get_duty( rgb_ctrl->ledc->channels + rgb_ctrl->red.channel);
    rgb.g = rgb_ctrl->ledc->get_duty( rgb_ctrl->ledc->channels + rgb_ctrl->green.channel);
    rgb.b = rgb_ctrl->ledc->get_duty( rgb_ctrl->ledc->channels + rgb_ctrl->blue.channel);

    sprintf(data, color_box_data_start
                                , rgb.r
                                , rgb.g
                                , rgb.b
                                ); 
httpd_resp_sendstr_chunk(req, data);
    // print sliders
    //strcpy(data+strlen(data), html_block_led_control_data_start);
httpd_resp_sendstr_chunk(req, html_block_data_header_start);

    rgb_to_hsv(&rgb, &rgb_ctrl->hsv);
    ledcontrol_channel_t *ch1, *ch2, *ch3;
    // print red channel  ( +100, чтобы не пересекатьс с уже существующими слайдерами, меняем name=ledc1 на ledc101 и т.д., и id=ledc1 на ledc=101)
    ch1 = rgb_ctrl->ledc->channels + rgb_ctrl->red.channel;
    sprintf( data, html_block_led_control_item
                                , ch1->name  // Красный
                                , ch1->channel + 100                                   // channel num
                                , ch1->duty              // channel duty    
                                , ch1->channel     // for data-uri                                  
                                , ch1->channel  + 100                                 // channel num
                                , ch1->duty              // channel duty
                                );    
    httpd_resp_sendstr_chunk(req, data);
    // print gree channel
    ch2 = rgb_ctrl->ledc->channels + rgb_ctrl->green.channel;
    sprintf( data, html_block_led_control_item
                                , ch2->name  // Красный
                                , ch2->channel + 100                                   // channel num
                                , ch2->duty              // channel duty    
                                , ch2->channel     // for data-uri                                  
                                , ch2->channel + 100                                  // channel num
                                , ch2->duty              // channel duty
                                );   
httpd_resp_sendstr_chunk(req, data);                                
    // print blue channel
    ch3 = rgb_ctrl->ledc->channels + rgb_ctrl->blue.channel;
    sprintf( data, html_block_led_control_item
                                , ch3->name  // Красный
                                , ch3->channel  + 100                                 // channel num
                                , ch3->duty              // channel duty    
                                , ch3->channel     // for data-uri                                  
                                , ch3->channel  + 100                                 // channel num
                                , ch3->duty              // channel duty
                                );   
    
httpd_resp_sendstr_chunk(req, data);

    //strcpy(data+strlen(data), html_block_led_control_end);
    httpd_resp_sendstr_chunk(req, html_block_data_end);
    //strcpy(data+strlen(data), html_block_led_control_end);
    httpd_resp_sendstr_chunk(req, html_block_data_end);
}


static void rgbcontrol_print_data(http_args_t *args)
{
    
    http_args_t *arg = (http_args_t *)args;
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)arg->dev;
    httpd_req_t *req = (httpd_req_t *)arg->req;

    #ifdef CONFIG_RGB_EFFECTS
    effects_t *ee = rgb_ctrl->effects;
    if ( ee == NULL ) return;    
    #endif



    // print RGB sliders block with color box
    char *buf = calloc(1024, sizeof(char));
    rgbcontrol_print_color_sliders(req, buf, rgb_ctrl);

    
    // print Color effect block with select

    //memset(buf, 1024, 0);
    //strcat(data, html_block_rgb_control_start);
    httpd_resp_sendstr_chunk(req, html_block_rgb_control_start);

    #ifdef CONFIG_RGB_EFFECTS
    //strcat(data, effects_select_start);
    httpd_resp_sendstr_chunk(req, effects_select_start);
    for (int i=0; i < COLOR_EFFECTS_MAX; i++ ) 
    {
        effect_t *e = ee->effect + i;
        memset(buf, 0, 1024);
        sprintf(buf, effects_item
                                        , i
                                        , (ee->effect_id == i || ( i == COLOR_EFFECTS_MAX-1 && ee->effect_id == -1) ) ? "selected=\"selected\" " : ""
                                        , e->name);
        httpd_resp_sendstr_chunk(req, buf);
    }
    //strcat(data, effects_select_end);
    httpd_resp_sendstr_chunk(req, effects_select_end);
    effect_t *e = ee->effect + ee->effect_id;
    #endif

    //strcat(data, html_block_rgb_control_end);
    httpd_resp_sendstr_chunk(req, html_block_rgb_control_end);
free(buf);
}

void rgbcontrol_register_http_print_data(rgbcontrol_handle_t dev_h)
{
    http_args_t *p = calloc(1,sizeof(http_args_t));
    p->dev = dev_h;
    register_print_page_block( "rgb", PAGES_URI[ PAGE_URI_ROOT], 7, rgbcontrol_print_data, p, NULL, NULL );
}

static esp_err_t http_process_rgb(httpd_req_t *req, char *param, size_t size)
{
    user_ctx_t *ctx = req->user_ctx;
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)ctx->args;

    esp_err_t err = http_get_key_str(req, "rgb", param, size);
    if (  err != ESP_OK ) {
        ESP_LOGE(TAG, "get key param of rgb ERROR");
        return err;
    }
    char *istr = strtok (param,",");
    color_rgb_t *rgb = malloc(sizeof(color_rgb_t));
    rgb->r = atoi(istr);
    istr = strtok (NULL,",");
    rgb->g = atoi(istr); 
    istr = strtok (NULL,",");
    rgb->b = atoi(istr);

    #ifdef CONFIG_RGB_EFFECTS
    effects_t *ef = (effects_t *) rgb_ctrl->effects;
    if ( ef != NULL ) ef->stop();
    #endif

    rgb_ctrl->set_color_rgb(*rgb);
    free(rgb);                
    return ESP_OK;
}

static esp_err_t http_process_rgb2(httpd_req_t *req)
{
    user_ctx_t *ctx = req->user_ctx;
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)ctx->args;

    esp_err_t err = ESP_FAIL;
    color_rgb_t *rgb = malloc(sizeof(color_rgb_t));
    if ( http_get_key_uint8(req, "r", &rgb->r) == ESP_OK &&
         http_get_key_uint8(req, "g", &rgb->g) == ESP_OK &&
         http_get_key_uint8(req, "b", &rgb->b) == ESP_OK) 
    {

        #ifdef CONFIG_RGB_EFFECTS
        effects_t *ef = (effects_t *) rgb_ctrl->effects;
        if ( ef != NULL ) ef->stop();
        #endif

        rgb_ctrl->set_color_rgb(*rgb);
        err = ESP_OK;
    }
    free(rgb);
    return err;
}

static esp_err_t http_process_hsv(httpd_req_t *req, char *param, size_t size) 
{
    user_ctx_t *ctx = req->user_ctx;
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)ctx->args;

    esp_err_t err = http_get_key_str(req, "hsv", param, size);
    if (  err != ESP_OK ) {
        ESP_LOGE(TAG, "get key param of hsv ERROR");
        return err;
    }
    char *istr = strtok (param,",");
    color_hsv_t *hsv = malloc( sizeof(color_hsv_t));
    hsv->h = atoi(istr);
    istr = strtok (NULL,",");
    hsv->s = atoi(istr);
    istr = strtok (NULL,",");
    hsv->v = atoi(istr);

    #ifdef CONFIG_RGB_EFFECTS
    effects_t *ef = (effects_t *) rgb_ctrl->effects;
    if ( ef != NULL ) ef->stop();
    #endif

    rgb_ctrl->set_color_hsv(*hsv);
    err = ESP_OK;
    free(hsv);
    return ESP_OK;
}

static esp_err_t http_process_hsv2(httpd_req_t *req) 
{
    user_ctx_t *ctx = req->user_ctx;
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)ctx->args;

    esp_err_t err = ESP_FAIL;
    color_hsv_t *hsv = malloc( sizeof(color_hsv_t));
    if ( http_get_key_uint16(req, "h", &hsv->h) == ESP_OK &&
            http_get_key_uint8(req, "s", &hsv->s) == ESP_OK &&
            http_get_key_uint8(req, "v", &hsv->v) == ESP_OK) 
    {
        #ifdef CONFIG_RGB_EFFECTS
        effects_t *ef = (effects_t *) rgb_ctrl->effects;
        if ( ef != NULL ) ef->stop();
        #endif

        rgb_ctrl->set_color_hsv(*hsv);
        err = ESP_OK;
    }
    free(hsv);
    return err;
}

static esp_err_t http_process_hex(httpd_req_t *req, char *param, size_t size) 
{
    
    user_ctx_t *ctx = req->user_ctx;
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)ctx->args;
    
    esp_err_t err = http_get_key_str(req, "val", param, size);
    if ( err != ESP_OK ) return err;
    
    #ifdef CONFIG_RGB_EFFECTS
    effects_t *ef = (effects_t *) rgb_ctrl->effects;
    if ( ef != NULL ) ef->stop();     
    #endif

    rgb_ctrl->set_color_hex(param);
    return ESP_OK;
}

static esp_err_t http_process_int(httpd_req_t *req) 
{
    user_ctx_t *ctx = req->user_ctx;
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)ctx->args;

    long color;
    esp_err_t err = http_get_key_long(req, "val", &color);
    if ( err != ESP_OK ) return err;

    #ifdef CONFIG_RGB_EFFECTS
    effects_t *ef = (effects_t *) rgb_ctrl->effects;
    if ( ef != NULL ) ef->stop();       
    #endif
    
    rgb_ctrl->set_color_int(color);
    return ESP_OK;
}

esp_err_t rgbcontrol_get_handler(httpd_req_t *req)
{
    // handle http get request
    char page[200] = "";
    esp_err_t err = ESP_FAIL;

    if ( http_get_has_params(req) == ESP_OK) 
    {
        user_ctx_t *ctx = req->user_ctx;
        rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)ctx->args;

        char param[30];

        if ( http_get_key_str(req, "rgb", param, sizeof(param)) == ESP_OK ) 
        {
            //  rgb?rgb=r,g,b  
            err = http_process_rgb(req, param, sizeof(param));
        }
        else if ( http_get_key_str(req, "hsv", param, sizeof(param)) == ESP_OK ) 
        {
            // rgb?hsv=h,s,v
            err = http_process_hsv(req, param, sizeof(param));
        }    
        else if ( http_get_key_str(req, "type", param, sizeof(param)) == ESP_OK ) 
        {
            if ( strcmp(param, "rgb") == ESP_OK ) 
            {
                err = http_process_rgb2(req); // rgb?type=rgb&r=r&g=g&b=b
            } 
            else if ( strcmp(param, "hsv") == ESP_OK ) 
            {
                err = http_process_hsv2(req); // rgb?type=hsv&h=h&s=s&v=v
            } 
            else if ( strcmp(param, "int") == ESP_OK ) 
            {
                err = http_process_int(req); // rgb?type=int&val=value
            } 
            else if ( strcmp(param, "hex") == ESP_OK ) 
            {
                err = http_process_hex(req, param, sizeof(param));  // rgb?type=hex&val=value
            } 
            #ifdef CONFIG_RGB_EFFECTS
            else if ( strcmp(param, "effect") == ESP_OK ) 
            {
                if ( http_get_key_str(req, "id", param, sizeof(param)) == ESP_OK ) 
                {
                    effects_t *ef = (effects_t *) rgb_ctrl->effects;
                    if ( ef != NULL ) {
                        ef->set( atoi(param) );
                    }    
                } 
                else if ( http_get_key_str(req, "name", param, sizeof(param)) == ESP_OK ) 
                {
                    effects_t *ef = (effects_t *) rgb_ctrl->effects;
                    if ( ef != NULL )                   
                        ef->set_by_name( param );
                }
            }
            #endif
        }

        strcpy( page, (err == ESP_OK ) ? "OK" : "ERROR");  

    } 

    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
    httpd_resp_send(req, page, strlen(page));
    return ESP_OK;        
}

void rgbcontrol_register_http_handler(httpd_handle_t _server, rgbcontrol_handle_t dev_h)
{
    user_ctx_t *ctx = (user_ctx_t *)calloc(1, sizeof(user_ctx_t));
    ctx->args = dev_h;
    add_uri_get_handler( _server, RGB_CONTROL_URI, rgbcontrol_get_handler, ctx); 
}

void rgbcontrol_http_init(httpd_handle_t _server, rgbcontrol_handle_t dev_h)
{
    rgbcontrol_register_http_print_data(dev_h);
    rgbcontrol_register_http_handler(_server, dev_h);
}


#endif