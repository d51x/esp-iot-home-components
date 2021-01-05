#include "rgbcontrol_http.h"
#include "http_page_tpl.h"

#ifdef CONFIG_RGB_CONTROLLER_HTTP

static const char* TAG = "RGBHTTP";

const char *html_block_rgb_control_title ICACHE_RODATA_ATTR = "RGB Controller";
static const char *rgb_options_title ICACHE_RODATA_ATTR = "RGB channels Config";
static const char *rgb_options_red_channel_title ICACHE_RODATA_ATTR = "Red channel";
static const char *rgb_options_green_channel_title ICACHE_RODATA_ATTR = "Green channel";
static const char *rgb_options_blue_channel_title ICACHE_RODATA_ATTR = "Blue channel";

extern const char *html_block_rgb_control_start ICACHE_RODATA_ATTR = 
    "<div class='group rnd'>"
        //"<h4 class='brd-btm'>Color Effects:</h4>";      
        "<h4 >Color Effects:</h4>";      

const char *html_block_rgb_control_end ICACHE_RODATA_ATTR =  "</div>";    

const char *color_box_data_start ICACHE_RODATA_ATTR = 
        "<div id='colors' style='background:rgb(%d,%d,%d)'></div>";

const char *effects_data_select_item ICACHE_RODATA_ATTR = "</div>";

 #ifdef CONFIG_RGB_EFFECTS
const char *effects_select_start ICACHE_RODATA_ATTR =  "<div class='ef'>"
                                                            "<select id=\"effects\" onchange=\"effects()\">";

    
#define effects_select_end html_select_end

#define effects_item html_select_item
#endif



#define effects_data_end html_block_data_end

const char *rgb_param_red_channel ICACHE_RODATA_ATTR = "redch";
const char *rgb_param_green_channel ICACHE_RODATA_ATTR = "greench";
const char *rgb_param_blue_channel ICACHE_RODATA_ATTR = "bluech";

const char *rgb_param_rgb ICACHE_RODATA_ATTR = "rgb";
const char *rgb_param_hsv ICACHE_RODATA_ATTR = "hsv";
const char *rgb_param_int ICACHE_RODATA_ATTR = "int";
const char *rgb_param_hex ICACHE_RODATA_ATTR = "hex";
const char *rgb_param_effect ICACHE_RODATA_ATTR = "effect";
const char *rgb_param_name ICACHE_RODATA_ATTR = "name";
const char *rgb_param_type ICACHE_RODATA_ATTR = "type";
const char *rgb_param_val ICACHE_RODATA_ATTR = "val";


static void rgbcontrol_print_color_sliders(httpd_req_t *req, rgbcontrol_t *rgb_ctrl)
{
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, html_block_rgb_control_title);

    // print color box
    color_rgb_t rgb;

    rgb.r = rgb_ctrl->ledc->get_duty( rgb_ctrl->ledc->channels + rgb_ctrl->red.channel);
    rgb.g = rgb_ctrl->ledc->get_duty( rgb_ctrl->ledc->channels + rgb_ctrl->green.channel);
    rgb.b = rgb_ctrl->ledc->get_duty( rgb_ctrl->ledc->channels + rgb_ctrl->blue.channel);
    httpd_resp_sendstr_chunk_fmt(req, color_box_data_start, rgb.r, rgb.g, rgb.b);
    
    // print sliders
    rgb_to_hsv(&rgb, &rgb_ctrl->hsv);
    ledcontrol_channel_t *ch1, *ch2, *ch3;

    // print red channel  ( +100, чтобы не пересекатьс с уже существующими слайдерами, меняем name=ledc1 на ledc101 и т.д., и id=ledc1 на ledc=101)
    ch1 = rgb_ctrl->ledc->channels + rgb_ctrl->red.channel;
    httpd_resp_sendstr_chunk_fmt(req, html_block_led_control_item
                                , ch1->name  // Красный
                                , 100 //ch1->channel + 100                                   // channel num
                                , ch1->duty              // channel duty    
                                , ch1->channel     // for data-uri                                  
                                , 100 //ch1->channel  + 100                                 // channel num
                                , ch1->duty              // channel duty
                                ); 

    // print gree channel
    ch2 = rgb_ctrl->ledc->channels + rgb_ctrl->green.channel;
    httpd_resp_sendstr_chunk_fmt(req, html_block_led_control_item
                                , ch2->name  // Красный
                                , 101 //ch2->channel + 100                                   // channel num
                                , ch2->duty              // channel duty    
                                , ch2->channel     // for data-uri                                  
                                , 101 //ch2->channel  + 100                                 // channel num
                                , ch2->duty              // channel duty
                                );                                
    // print blue channel
    ch3 = rgb_ctrl->ledc->channels + rgb_ctrl->blue.channel;
    httpd_resp_sendstr_chunk_fmt(req, html_block_led_control_item
                                , ch3->name  // Красный
                                , 102 //ch3->channel + 100                                   // channel num
                                , ch3->duty              // channel duty    
                                , ch3->channel     // for data-uri                                  
                                , 102 //ch3->channel  + 100                                 // channel num
                                , ch3->duty              // channel duty
                                );  

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
    rgbcontrol_print_color_sliders(req, rgb_ctrl);

    
    // print Color effect block with select
    httpd_resp_sendstr_chunk(req, html_block_rgb_control_start);

    #ifdef CONFIG_RGB_EFFECTS
        httpd_resp_sendstr_chunk(req, effects_select_start);
        for (int i=0; i < COLOR_EFFECTS_MAX; i++ ) 
        {
            effect_t *e = ee->effect;
            httpd_resp_sendstr_chunk_fmt(req, effects_item
                                            , i
                                            , (ee->effect_id == i || ( i == COLOR_EFFECTS_MAX-1 && ee->effect_id == -1) ) ? html_selected : ""
                                            , color_effects[i].name);
        }

        httpd_resp_sendstr_chunk(req, effects_select_end);
        httpd_resp_sendstr_chunk(req, html_block_data_end);
    #endif

    //strcat(data, html_block_rgb_control_end);
    httpd_resp_sendstr_chunk(req, html_block_rgb_control_end);
}

static void rgbcontrol_print_options(http_args_t *args)
{
    httpd_req_t *req = (httpd_req_t *)args->req;
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)args->dev;

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_header_start, rgb_options_title);
    // ==========================================================================    
    // print form with options
    httpd_resp_sendstr_chunk(req, html_block_data_form_start);

    char value[4];

    // Red channel
    itoa(rgb_ctrl->red.channel, value, 10);
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                    , rgb_options_red_channel_title // %s label
                                    , rgb_param_red_channel   // %s name
                                    , value   // %d value
    );    

    //green
    itoa(rgb_ctrl->green.channel, value, 10);
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                    , rgb_options_green_channel_title // %s label
                                    , rgb_param_green_channel   // %s name
                                    , value   // %d value
    ); 

    //blue
    itoa(rgb_ctrl->blue.channel, value, 10);
    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_item_label_edit
                                    , rgb_options_blue_channel_title // %s label
                                    , rgb_param_blue_channel   // %s name
                                    , value   // %d value
    ); 

    httpd_resp_sendstr_chunk_fmt(req, html_block_data_form_submit, rgb_param_rgb);
    httpd_resp_sendstr_chunk(req, html_block_data_form_end);
    // ==========================================================================    
    //httpd_resp_sendstr_chunk(req, html_block_data_end);    
    httpd_resp_sendstr_chunk(req, html_block_data_end); 

}

static void rgbcontrol_process_params(httpd_req_t *req, void *args)
{
    ESP_LOGI( TAG, LOG_FMT());
    
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)((http_args_t *)args)->dev;

	if ( http_get_has_params(req) == ESP_OK) 
	{
        char param[50];
        if ( http_get_key_str(req, "st", param, sizeof(param)) == ESP_OK ) 
        {
            if ( strcmp(param, rgb_param_rgb) == 0 ) 
            {
                // продолжим обработку параметров    
                http_get_key_uint8(req, rgb_param_red_channel, &rgb_ctrl->red.channel);
                http_get_key_uint8(req, rgb_param_green_channel, &rgb_ctrl->green.channel);
                http_get_key_uint8(req, rgb_param_blue_channel, &rgb_ctrl->blue.channel);

                rgb_ctrl->red = rgb_ctrl->ledc->channels[rgb_ctrl->red.channel];
                rgb_ctrl->green = rgb_ctrl->ledc->channels[rgb_ctrl->green.channel];
                rgb_ctrl->blue = rgb_ctrl->ledc->channels[rgb_ctrl->blue.channel];

                ledcontrol_channel_set_name(&rgb_ctrl->red, "Red");
                ledcontrol_channel_set_name(&rgb_ctrl->green, "Green");
                ledcontrol_channel_set_name(&rgb_ctrl->blue, "Blue");

                // save to nvs
                rgbcontrol_save_nvs(rgb_ctrl);
            }
        }
    }    
}

void rgbcontrol_register_http_print_data(rgbcontrol_handle_t dev_h)
{
    http_args_t *p = calloc(1,sizeof(http_args_t));
    p->dev = dev_h;
    register_print_page_block( rgb_param_rgb, PAGES_URI[ PAGE_URI_ROOT], 7, rgbcontrol_print_data, p, NULL, NULL );
    #ifdef CONFIG_PAGE_TOOLS
    register_print_page_block( "rgb_opt", PAGES_URI[ PAGE_URI_TOOLS], 4, rgbcontrol_print_options, p, rgbcontrol_process_params, p );
    #endif
}

static esp_err_t http_process_rgb(httpd_req_t *req, char *param, size_t size)
{
    user_ctx_t *ctx = req->user_ctx;
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)ctx->args;

    esp_err_t err = http_get_key_str(req, rgb_param_rgb, param, size);
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

    esp_err_t err = http_get_key_str(req, rgb_param_hsv, param, size);
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
    
    esp_err_t err = http_get_key_str(req, rgb_param_val, param, size);
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
    esp_err_t err = http_get_key_long(req, rgb_param_val, &color);
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

        if ( http_get_key_str(req, rgb_param_rgb, param, sizeof(param)) == ESP_OK ) 
        {
            //  rgb?rgb=r,g,b  
            err = http_process_rgb(req, param, sizeof(param));
        }
        else if ( http_get_key_str(req, rgb_param_hsv, param, sizeof(param)) == ESP_OK ) 
        {
            // rgb?hsv=h,s,v
            err = http_process_hsv(req, param, sizeof(param));
        }    
        else if ( http_get_key_str(req, rgb_param_type, param, sizeof(param)) == ESP_OK ) 
        {
            if ( strcmp(param, rgb_param_rgb) == ESP_OK ) 
            {
                err = http_process_rgb2(req); // rgb?type=rgb&r=r&g=g&b=b
            } 
            else if ( strcmp(param, rgb_param_hsv) == ESP_OK ) 
            {
                err = http_process_hsv2(req); // rgb?type=hsv&h=h&s=s&v=v
            } 
            else if ( strcmp(param, rgb_param_int) == ESP_OK ) 
            {
                err = http_process_int(req); // rgb?type=int&val=value
            } 
            else if ( strcmp(param, rgb_param_hex) == ESP_OK ) 
            {
                err = http_process_hex(req, param, sizeof(param));  // rgb?type=hex&val=value
            } 
            #ifdef CONFIG_RGB_EFFECTS
            else if ( strcmp(param, rgb_param_effect) == ESP_OK ) 
            {
                if ( http_get_key_str(req, "id", param, sizeof(param)) == ESP_OK ) 
                {
                    effects_t *ef = (effects_t *) rgb_ctrl->effects;
                    if ( ef != NULL ) {
                        ef->set( atoi(param) );
                    }    
                } 
                else if ( http_get_key_str(req, rgb_param_name, param, sizeof(param)) == ESP_OK ) 
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