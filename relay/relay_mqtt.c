#include "relay_mqtt.h"

#ifdef CONFIG_COMPONENT_RELAY

static const char* TAG = "RELAY";

static void relay_mqtt_periodic_send_cb(char **buf, void *args)
{
    // для отправки в buf положить значение пина
    relay_t *p = (relay_t *)args;
    itoa(p->state, *buf, 10);
}

void relay_mqtt_recv_cb(char *buf, void *args)
{
    relay_t *p = (relay_t *)args;
    uint8_t value = atoi( buf );

    #ifdef CONFIG_MQTT_TOPIC_SEND_RECV
        if ( p->prev != value )
        {       
            if ( relay_write( (relay_handle_t)p, value) == ESP_OK )
                p->prev = value;       
        }
    #else
        relay_write( (relay_handle_t)p, value);
    #endif 
}

static void relay_mqtt_recv_queue_cb(void *arg)
{
    const portTickType xTicksToWait = 100 / portTICK_RATE_MS;
    relay_t *dev = (relay_t *)calloc(1, sizeof(relay_t));

    for( ;; )
    {
        if ( relay_status_queue == NULL ) continue;
        if ( xQueueReceive( relay_status_queue, dev, xTicksToWait ) == pdPASS )
        {
            relay_t *relay = (relay_t *)dev;
            char t[20], v[2];
            sprintf( t, RELAY_MQTT_SEND_TOPIC, relay->pin);
            itoa(relay->state, v, 10);
            mqtt_publish(t, v);     
        }           
    }
}

void relay_mqtt_init()
{
    if ( relay_count > 0 && relays != NULL )
        xTaskCreate(relay_mqtt_recv_queue_cb, "relay_recv", 1024, NULL, 10, NULL); // При 1024 иногда случался Stack canary watchpoint triggered (mcp23017_recv)
    else
        ESP_LOGE(TAG, "%s: no relays initialized yet", __func__);

    for ( uint8_t i = 0; i < relay_count; i++)
    {
        char t[20];
        sprintf(t, RELAY_MQTT_SEND_TOPIC, relays[i].pin);
        mqtt_add_periodic_publish_callback( t, relay_mqtt_periodic_send_cb, (relay_t *)&relays[i]);

        sprintf(t, RELAY_MQTT_RECV_TOPIC, relays[i].pin);
        mqtt_add_receive_callback(t, 1, relay_mqtt_recv_cb, (relay_t *)&relays[i]);
        //free(p);    make after in delete callback
    }    
}

#endif
