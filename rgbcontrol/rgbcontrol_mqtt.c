#include "rgbcontrol_mqtt.h"

#ifdef CONFIG_RGB_CONTROLLER

static const char* TAG = "RGBMQTT";

#define MQTT_SEND_TOPIC_COLOR_INT "color/int"
#define MQTT_SEND_TOPIC_EFFECT_ID "effect/id"
#define MQTT_SEND_TOPIC_EFFECT_NAME "effect/name"

#ifdef CONFIG_MQTT_TOPIC_SEND_RECV
#define MQTT_RECV_TOPIC_COLOR_INT MQTT_SEND_TOPIC_COLOR_INT
#define MQTT_RECV_TOPIC_EFFECT_ID MQTT_SEND_TOPIC_EFFECT_ID
#define MQTT_RECV_TOPIC_EFFECT_NAME MQTT_SEND_TOPIC_EFFECT_NAME
#else
#define MQTT_RECV_TOPIC_COLOR_INT MQTT_SEND_TOPIC_COLOR_INT"/set"
#define MQTT_RECV_TOPIC_EFFECT_ID MQTT_SEND_TOPIC_EFFECT_ID"/set"
#define MQTT_RECV_TOPIC_EFFECT_NAME MQTT_SEND_TOPIC_EFFECT_NAME"set"
#endif

static void rgbcontrol_mqtt_recv_queue_cb(void *arg)
{
    const portTickType xTicksToWait = 100 / portTICK_RATE_MS;
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)arg;

    rgbcontrol_queue_t *data = (rgbcontrol_queue_t *) calloc(1, sizeof(rgbcontrol_queue_t));
    char payload[20];

    for( ;; )
    {
        if ( rgbcontrol_color_queue != NULL && xQueueReceive( rgbcontrol_color_queue, data, xTicksToWait ) == pdPASS )
        {
            
            if ( data->type == RGB_COLOR_INT)
            {
                itoa(data->data, payload, 10);
                mqtt_publish(MQTT_SEND_TOPIC_COLOR_INT, payload);
            } 
            
            #ifdef CONFIG_RGB_EFFECTS
            else if (data->type == RGB_EFFECT_ID )
            {
                itoa(data->data, payload, 10);
                mqtt_publish(MQTT_SEND_TOPIC_EFFECT_ID, payload);                
            }
            else if ( data->type == RGB_EFFECT_NAME)
            {
                mqtt_publish(MQTT_SEND_TOPIC_EFFECT_NAME, (char *)data>data);  
            }
            #endif
        }  
    }    
}

static void rgbcontrol_mqtt_periodic_send_color_int_cb(char **buf, void *args)
{
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)args;

    uint32_t color32 = 0;
    color_rgb_t *rgb = malloc(sizeof(color_rgb_t));
    hsv_to_rgb(rgb, rgb_ctrl->hsv);
    rgb_to_int(rgb, &color32);
    itoa( color32, *buf, 10);
    free(rgb);
}


#ifdef CONFIG_RGB_EFFECTS
static void rgbcontrol_mqtt_periodic_send_effect_id_cb(char **buf, void *args)
{
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)args;
    effects_t *ef = (effects_t *) rgb_ctrl->effects;
    itoa(ef->effect_id, *buf, 10);
}

static void rgbcontrol_mqtt_periodic_send_effect_name_cb(char **buf, void *args)
{
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)args;
    effects_t *ef = (effects_t *) rgb_ctrl->effects;    
    strcpy(*buf, ef->effect->name);
}
#endif

static void rgbcontrol_mqtt_recv_color_int_cb(char *buf, void *args)
{
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)args;
    uint32_t color32 = atoi(buf);
    static uint32_t prev = 0;
    
    if ( color32 != prev )
    {   
        #ifdef CONFIG_RGB_EFFECTS
        effects_t *ef = (effects_t *) rgb_ctrl->effects;
        if ( ef != NULL ) ef->stop(); 
        #endif

        rgb_ctrl->set_color_int(color32); 
        prev = color32;
    }
}


#ifdef CONFIG_RGB_EFFECTS
static void rgbcontrol_mqtt_recv_effect_id_cb(char *buf, void *args)
{
    rgbcontrol_t *rgb_ctrl = (rgbcontrol_t *)args;
    effects_t *ef = (effects_t *) rgb_ctrl->effects;
    static uint8_t prev = 0;
    uint8_t val = atoi(buf);
    if ( val != prev )
    {
        ef->set( val );
        prev = val;
    }
    
}
#endif

void rgbcontrol_mqtt_init(rgbcontrol_handle_t dev_h)
{

    xTaskCreate(rgbcontrol_mqtt_recv_queue_cb, "rgbctrl", 1024, dev_h, 13, NULL);

    //mqtt_add_periodic_publish_callback( MQTT_SEND_TOPIC_COLOR_INT, rgbcontrol_mqtt_periodic_send_color_int_cb, dev_h); 

    
    #ifdef CONFIG_RGB_EFFECTS
    //mqtt_add_periodic_publish_callback( MQTT_SEND_TOPIC_EFFECT_ID, rgbcontrol_mqtt_periodic_send_effect_id_cb, dev_h); 
    //mqtt_add_periodic_publish_callback( MQTT_SEND_TOPIC_EFFECT_NAME, rgbcontrol_mqtt_periodic_send_effect_name_cb, dev_h); 
    #endif

    mqtt_add_receive_callback(MQTT_RECV_TOPIC_COLOR_INT, 1, rgbcontrol_mqtt_recv_color_int_cb, dev_h);  
    
    #ifdef CONFIG_RGB_EFFECTS
    mqtt_add_receive_callback(MQTT_RECV_TOPIC_EFFECT_ID, 1, rgbcontrol_mqtt_recv_effect_id_cb, dev_h);  
    #endif
}

#endif