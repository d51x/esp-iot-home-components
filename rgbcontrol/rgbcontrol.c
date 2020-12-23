#include "rgbcontrol.h"

#ifdef CONFIG_RGB_CONTROLLER

static const char *TAG = "RGB";

volatile static rgbcontrol_t *rgb_ctrl = NULL;

void rgbcontrol_set_color_hsv(color_hsv_t hsv);  
void rgbcontrol_set_color_rgb(color_rgb_t rgb);  
void rgbcontrol_set_color_int(uint32_t color32);  
void rgbcontrol_set_color_hex(const char *hex);  

void rgbcontrol_set_brightness(int8_t value);
void rgbcontrol_fade_brightness(int8_t brightness_from, int8_t brightness_to, int16_t brightness_delay);
void rgbcontrol_inc_brightness(int8_t step);
void rgbcontrol_dec_brightness(int8_t step);

void rgbcontrol_set_saturation(int8_t value);
void rgbcontrol_fade_saturation(int8_t saturation_from, int8_t saturation_to, int16_t saturation_delay);
void rgbcontrol_inc_saturation(int8_t step);
void rgbcontrol_dec_saturation(int8_t step);

#ifdef CONFIG_RGB_EFFECTS
void rgbcontrol_set_effects(effects_t *effects);
#endif

void _rgbcontrol_set_color_rgb(color_rgb_t rgb, bool update);
void _rgbcontrol_set_color_hsv(color_hsv_t hsv, bool update);



rgbcontrol_t* rgbcontrol_init(ledcontrol_t *ledc, ledcontrol_channel_t *red, ledcontrol_channel_t *green, ledcontrol_channel_t *blue)
{
    if ( ledc == NULL ) {
        ESP_LOGE(TAG, "LED Controller is not initialized yet.");
        return NULL;
    }
    
    if ( red == NULL || green == NULL || blue == NULL ) {
        ESP_LOGE(TAG, "One of color channels is NULL");
        return NULL;
    }

    rgb_ctrl = calloc(1, sizeof(rgbcontrol_t));
    rgb_ctrl->ledc = ledc;

    rgb_ctrl->red = *red;
    rgb_ctrl->green = *green;
    rgb_ctrl->blue = *blue;

    rgb_ctrl->hsv.h = 0;
    rgb_ctrl->hsv.s = 0;
    rgb_ctrl->hsv.v = 0;

    rgb_ctrl->fade_delay = RGB_DEFAULT_FADE;
    rgb_ctrl->fadeup_delay = RGB_DEFAULT_FADEUP;
    rgb_ctrl->fadedown_delay = RGB_DEFAULT_FADEDOWN;

	// указатели на функции
	rgb_ctrl->set_color_hsv = rgbcontrol_set_color_hsv;
	rgb_ctrl->set_color_rgb = rgbcontrol_set_color_rgb;
	rgb_ctrl->set_color_int = rgbcontrol_set_color_int;
	rgb_ctrl->set_color_hex = rgbcontrol_set_color_hex;

    rgb_ctrl->set_brightness = rgbcontrol_set_brightness;
    rgb_ctrl->fade_brightness = rgbcontrol_fade_brightness;
    rgb_ctrl->inc_brightness = rgbcontrol_inc_brightness;
    rgb_ctrl->dec_brightness = rgbcontrol_dec_brightness;

    rgb_ctrl->set_saturation = rgbcontrol_set_saturation;
    rgb_ctrl->fade_saturation = rgbcontrol_fade_saturation;
    rgb_ctrl->inc_saturation = rgbcontrol_inc_saturation;
    rgb_ctrl->dec_saturation = rgbcontrol_dec_saturation;

    #ifdef CONFIG_RGB_EFFECTS
    rgb_ctrl->effect_id = COLOR_EFFECTS_MAX-1;
    rgb_ctrl->set_effects = rgbcontrol_set_effects;    
    #endif

    rgbcontrol_color_mqtt_send_queue = xQueueCreate(5, sizeof(rgbcontrol_queue_t));

    return rgb_ctrl;
}

void _rgbcontrol_set_color_hsv(color_hsv_t hsv, bool update) {
    color_rgb_t *rgb = malloc( sizeof(color_rgb_t));
	hsv_to_rgb(rgb, hsv);
    _rgbcontrol_set_color_rgb(*rgb, update);
    memcpy(&rgb_ctrl->hsv, &hsv, sizeof(color_hsv_t));
	free(rgb);
}

void rgbcontrol_set_color_hsv(color_hsv_t hsv) {
    _rgbcontrol_set_color_hsv(hsv, true);
}

void _rgbcontrol_set_color_rgb(color_rgb_t rgb, bool update) {
    rgb_to_hsv(&rgb, &rgb_ctrl->hsv);
    rgb_ctrl->ledc->set_duty( rgb_ctrl->ledc->channels + rgb_ctrl->red.channel,   rgb.r );
    rgb_ctrl->ledc->set_duty( rgb_ctrl->ledc->channels + rgb_ctrl->green.channel, rgb.g );
    rgb_ctrl->ledc->set_duty( rgb_ctrl->ledc->channels + rgb_ctrl->blue.channel,  rgb.b );
    if ( update ) rgb_ctrl->ledc->update();	  
}

void rgbcontrol_set_color_rgb(color_rgb_t rgb) {
    _rgbcontrol_set_color_rgb(rgb, true); 
}

void rgbcontrol_set_color_int(uint32_t color32) {
	color_rgb_t *rgb = malloc( sizeof(color_rgb_t));
	int_to_rgb( color32, rgb);
	rgbcontrol_set_color_rgb(*rgb);
	free(rgb);
    rgbcontrol_queue_t *data = (rgbcontrol_queue_t *) calloc(1, sizeof(rgbcontrol_queue_t));
    data->type = RGB_COLOR_INT;
    data->data = color32;
    if ( rgbcontrol_color_mqtt_send_queue != NULL ) 
        xQueueSendToBack(rgbcontrol_color_mqtt_send_queue, data, 0);
    free(data);
}

void rgbcontrol_set_color_hex(const char *hex) {
	color_rgb_t *rgb = malloc( sizeof(color_rgb_t));
	hex_to_rgb( hex, rgb);
	rgbcontrol_set_color_rgb(*rgb);
	free(rgb);
}

void rgbcontrol_set_brightness(int8_t value){
    if ( value > VAL_MAX ) rgb_ctrl->hsv.v = VAL_MAX;
    else if ( value < 0 ) rgb_ctrl->hsv.v = 0;
    else  rgb_ctrl->hsv.v = value;
    rgbcontrol_set_color_hsv( rgb_ctrl->hsv );
}

void rgbcontrol_inc_brightness(int8_t step){
    int8_t val = rgb_ctrl->hsv.v;
    if ( val + step > VAL_MAX ) val = VAL_MAX;
    else val += step;
    rgbcontrol_set_brightness( val );
}

void rgbcontrol_dec_brightness(int8_t step){
    int8_t val = rgb_ctrl->hsv.v;
    if ( val - step < 0 ) val = 0;
    else val -= step;
    rgbcontrol_set_brightness( val );
}

void rgbcontrol_set_saturation(int8_t value){
    if ( value > SAT_MAX ) rgb_ctrl->hsv.s = SAT_MAX;
    else if ( value < 0 ) rgb_ctrl->hsv.s = 0;
    else rgb_ctrl->hsv.s = value;
    rgbcontrol_set_color_hsv( rgb_ctrl->hsv );
}

void rgbcontrol_inc_saturation(int8_t step){;
    int8_t val = rgb_ctrl->hsv.s;
    if ( val + step > SAT_MAX ) val = SAT_MAX;
    else val += step;
    rgbcontrol_set_saturation( val );
}

void rgbcontrol_dec_saturation(int8_t step){
    int8_t val = rgb_ctrl->hsv.s;
    if ( val - step < 0 ) val = 0;
    else val -= step;
    rgbcontrol_set_saturation( val );
}

void rgbcontrol_fade_brightness(int8_t brightness_from, int8_t brightness_to, int16_t brightness_delay){
    direction_e direction = (brightness_from < brightness_to) ? UP : DOWN;
    int8_t brightness = brightness_from;

    while ( 
            ((direction == UP) && ( brightness <= brightness_to)) ||
            ((direction == DOWN) && ( brightness >= brightness_to))
          )  
    {
        rgbcontrol_set_brightness( brightness );
        if ( direction == UP )
            brightness++;   // TODO: учесть brightness table
        else
            brightness--;  // TODO: учесть brightness table

        vTaskDelay( brightness_delay / portTICK_RATE_MS );
    }  
}

void rgbcontrol_fade_saturation(int8_t saturation_from, int8_t saturation_to, int16_t saturation_delay){
    direction_e direction = (saturation_from < saturation_to) ? UP : DOWN;
    int8_t saturation = saturation_from;
    while ( 
            ((direction == UP) && ( saturation <= saturation_to)) ||
            ((direction == DOWN) && ( saturation >= saturation_to))
          )  
    {
        rgbcontrol_set_saturation( saturation );
        if ( direction == UP )
            saturation++;   // TODO: учесть brightness table
        else
            saturation--;  // TODO: учесть brightness table

        vTaskDelay( saturation_delay / portTICK_RATE_MS );
    }  
}

#ifdef CONFIG_RGB_EFFECTS
void rgbcontrol_set_effects(effects_t *effects){
    rgb_ctrl->effects = effects;
}

void rgbcontrol_effects_init(rgbcontrol_t *rgbctrl, effects_t* effects)
{
    effects_init( rgbctrl, rgbctrl->set_color_hsv );
    rgbctrl->set_effects( effects );
}
#endif


#endif