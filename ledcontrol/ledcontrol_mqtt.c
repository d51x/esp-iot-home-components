#include "ledcontrol_mqtt.h"

#ifdef CONFIG_LED_CONTROLLER

static const char* TAG = "LEDCMQTT";

ledcontrol_handle_t _dev_h;

void ledcontrol_mqtt_periodic_send_cb(char **buf, void *args)
{
    // для отправки в buf положить значение пина
    ledcontrol_mqtt_t *p = (ledcontrol_mqtt_t *)args;
    ledcontrol_handle_t dev_h = p->dev_h;
    ledcontrol_t *ledc = (ledcontrol_t *)dev_h;
    ledcontrol_channel_t *ch = ledc->channels + p->channel;

    uint8_t value = 0;
    value = ledc->get_duty( ch );
    itoa(value, *buf, 10);
}

static esp_err_t ledcontrol_set_duty_update(ledcontrol_mqtt_t *p, uint8_t duty)
{
    esp_err_t err =  ESP_FAIL;
    if ( duty >= 0 && duty <= MAX_DUTY ) 
    {
        ledcontrol_handle_t dev_h = p->dev_h;
        ledcontrol_t *ledc = (ledcontrol_t *)dev_h;
        ledcontrol_channel_t *ch = ledc->channels + p->channel;
        err = ledc->set_duty( ch, duty );
        ledc->update();
    } 
    return err;
}

void ledcontrol_mqtt_recv_cb(char *buf, void *args)
{
    ledcontrol_mqtt_t *p = (ledcontrol_mqtt_t *)args;
    uint8_t value = atoi( buf );

    #ifdef CONFIG_MQTT_TOPIC_SEND_RECV
    if ( p->prev != value )
        {
            if ( ledcontrol_set_duty_update(p, value) == ESP_OK )
                p->prev = value;       
        }
    #else
        ledcontrol_set_duty_update(p, value);
    #endif 
}

void ledcontrol_mqtt_init(ledcontrol_handle_t dev_h)
{
    if ( dev_h == NULL ) return;
    ledcontrol_t *ledc = (ledcontrol_t *)dev_h;

    char t[20];

    for (uint8_t i = 0; i < ledc->led_cnt; i++ )
    {
        sprintf(t, LEDCONTROL_MQTT_SEND_TOPIC, i);
        mqtt_del_periodic_publish_callback(t, ledcontrol_mqtt_periodic_send_cb, NULL);
        mqtt_del_receive_callback(t, 1, ledcontrol_mqtt_recv_cb, NULL);
    }

    for (uint8_t i = 0; i < ledc->led_cnt; i++ ) {   
        ledcontrol_mqtt_t *p = (ledcontrol_mqtt_t *) calloc(1, sizeof(ledcontrol_mqtt_t));
        p->dev_h = dev_h;
        p->channel = i;

        sprintf(t, LEDCONTROL_MQTT_SEND_TOPIC, i);
        mqtt_add_periodic_publish_callback( t, ledcontrol_mqtt_periodic_send_cb, (ledcontrol_mqtt_t *)p); 

        sprintf(t, LEDCONTROL_MQTT_RECV_TOPIC, i);
        mqtt_add_receive_callback(t, 1, ledcontrol_mqtt_recv_cb, (ledcontrol_mqtt_t *)p);                
    }
}

#endif