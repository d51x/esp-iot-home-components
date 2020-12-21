
#include "relay.h"

#ifdef CONFIG_COMPONENT_RELAY

#define IOT_CHECK(tag, a, ret)  if(!(a)) {                                 \
        ESP_LOGE(tag,"%s:%d (%s)", __FILE__, __LINE__, __FUNCTION__);      \
        return (ret);                                                      \
        }
#define POINT_ASSERT(tag, param)	IOT_CHECK(tag, (param) != NULL, ESP_FAIL)




static const char* TAG = "RELAY";

QueueHandle_t relay_status_queue = NULL;
relay_t *relays = NULL;

relay_handle_t relay_create(char *name, gpio_num_t pin, relay_close_level_t level)
{   
    for (uint8_t i = 0; i < relay_count; i++)
    {
        if ( relays[i].pin == pin )
            return;
    }

    if ( relay_status_queue == NULL )
        relay_status_queue = xQueueCreate(5, sizeof(relay_t));

    relay_count++;
        relays = (relay_t * ) realloc( relays, relay_count * sizeof(relay_t));

    relay_t* relay_p = (relay_t*) calloc(1, sizeof(relay_t));
    relay_p->pin = pin;
    relay_p->name = name;
    relay_p->close_level = level;
    relay_p->state = RELAY_STATE_CLOSE;

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << pin;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    IOT_CHECK(TAG, gpio_config(&io_conf) == ESP_OK, NULL);

    memcpy(&relays[ relay_count - 1], relay_p, sizeof(relay_t));
    return (relay_handle_t) relay_p;
}

esp_err_t relay_write(relay_handle_t relay_handle, relay_state_t state)
{
    relay_t* relay = (relay_t*) relay_handle;
    POINT_ASSERT(TAG, relay_handle);

    gpio_set_level(relay->pin, (0x01 & state) ^ relay->close_level);
    relay->state = state;

    if ( relay_status_queue != NULL )
    {
        xQueueSendToBack(relay_status_queue, relay, 0);
    }
    
    return ESP_OK;
}

relay_state_t relay_read(relay_handle_t relay_handle)
{
    relay_t* relay = (relay_t*) relay_handle;
    return relay->state;
}

esp_err_t relay_delete(relay_handle_t relay_handle)
{
    POINT_ASSERT(TAG, relay_handle);
    relay_t *relay = (relay_t *) relay_handle;

    relay_t *r = (relay_t *) calloc( relay_count-1, sizeof(relay_t));
    uint8_t k = 0;
    for (uint8_t i = 0; i <relay_count; i++)
    {
        if (  relays[i].pin != relay->pin )
        {
            memcpy(&r[k], &relays[i], sizeof(relay_t));
            k++;
        }
    }
    relay_count--;
    relays = (relay_t *)realloc(relays, relay_count * sizeof(relay_t));
    memcpy(relays, r, relay_count * sizeof(relay_t));
    free(r);
    
    free(relay_handle);

    return ESP_OK;
}

#endif