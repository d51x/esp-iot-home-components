#include "mcp23017_mqtt.h"


static const char* TAG = "MCP23017";

mcp23017_handle_t _dev_h;

//mqtt_add_periodic_publish_callback( const char *topic, func_mqtt_send_cb fn_cb);
void mcp23017_mqtt_periodic_send_cb(char **buf, void *args)
{
    // для отправки в buf положить значение пина
    mcp23017_mqtt_t *p = (mcp23017_mqtt_t *)args;
    uint8_t value = 0;
    mcp23017_read_pin( p->dev_h, p->pin, &value);
    value = (value != 0);
    itoa(value, *buf, 10);
}

// void mqtt_add_receive_callback( const char *topic, func_mqtt_recv_cb fn_cb); 
void mcp23017_mqtt_recv_cb(char *buf, void *args)
{
    mcp23017_mqtt_t *p = (mcp23017_mqtt_t *)args;
    uint8_t value = atoi( buf );

    #ifdef CONFIG_MQTT_TOPIC_SEND_RECV
        if ( p->prev != value )
        {
            if ( mcp23017_write_pin( p->dev_h, p->pin, value) == ESP_OK )
                p->prev = value;       
        }
    #else
        mcp23017_write_pin( p->dev_h, p->pin, value);
    #endif 
}

void mcp23017_mqtt_recv_queue_cb(void *arg)
{
    const portTickType xTicksToWait = 100 / portTICK_RATE_MS;
    uint8_t data[2];

    mcp23017_handle_t dev_h = (mcp23017_handle_t)arg;

    for( ;; )
    {
        if ( xQueueReceive( mcp23017_status_queue, &data, xTicksToWait ) == pdPASS )
        {
            char t[20], v[2];
            sprintf( t, "%s%d", MCP23017_MQTT_SEND_TOPIC, data[0]);
            itoa(data[1], v, 10);
            mqtt_publish(t, v);     
        }           
    }
}

void mcp23017_mqtt_init(mcp23017_handle_t dev_h)
{
    xTaskCreate(mcp23017_mqtt_recv_queue_cb, "mcp23017_recv", 1024 + 256, dev_h, 10, NULL); // При 1024 иногда случался Stack canary watchpoint triggered (mcp23017_recv)

    _dev_h = dev_h;

    for ( uint8_t i = 0; i < 16; i++)
    {
        char t[20];
        
        mcp23017_mqtt_t *p = (mcp23017_mqtt_t *) calloc(1, sizeof(mcp23017_mqtt_t));
        p->dev_h = dev_h;
        p->pin = i;

        sprintf(t, MCP23017_MQTT_SEND_TOPIC, i);
        mqtt_add_periodic_publish_callback( t, mcp23017_mqtt_periodic_send_cb, (mcp23017_mqtt_t *)p);

        sprintf(t, MCP23017_MQTT_RECV_TOPIC, i);
        mqtt_add_receive_callback(t, 1, mcp23017_mqtt_recv_cb, (mcp23017_mqtt_t *)p);
        //free(p);    make after in delete callback
    }    
}
