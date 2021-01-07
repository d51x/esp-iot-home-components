
#include "relay.h"
#include "nvsparam.h"

#ifdef CONFIG_COMPONENT_RELAY

#ifndef IOT_CHECK
#define IOT_CHECK(tag, a, ret)  if(!(a)) {                                 \
        ESP_LOGE(tag,"%s:%d (%s)", __FILE__, __LINE__, __FUNCTION__);      \
        return (ret);                                                      \
        }
#endif

#ifndef POINT_ASSERT
#define POINT_ASSERT(tag, param)	IOT_CHECK(tag, (param) != NULL, ESP_FAIL)
#endif



static const char* TAG = "RELAY";

#define PARAM_RELAYS "relays"
//#define PARAM_RELAYS_COUNT "cnt"  // сохраняем кол-во
//#define PARAM_RELAYS_DATA "data"  // сохраняем целиком массив [relay_t *relays]

#define PARAM_RELAY_STATE "pinst%d"

QueueHandle_t relay_status_queue = NULL;
relay_t **relays = NULL;

static void relay_save_state(relay_t *relay)
{
    if ( relay->save ) {
        // get last state from nvs
        char s[8];
        sprintf(s, PARAM_RELAY_STATE, relay->pin);
        nvs_param_u8_save(PARAM_RELAYS, s, relay->state);
    }
}

relay_handle_t relay_create(const char *name, gpio_num_t pin, relay_level_t level, bool save)
{   
    for (uint8_t i = 0; i < relay_count; i++)
    {
        relay_t *r = relays[i];
        if ( r->pin == pin ) // уже есть
            return;
    }
    
    // for (uint8_t i = 0; i < relay_count; i++)
    // {
    //     ESP_LOGW(TAG, "%s: relays[%d] = %p", __func__, i, relays[i]);
    // }

    if ( relay_status_queue == NULL )
        relay_status_queue = xQueueCreate(5, sizeof(relay_t));


    relay_t **tmp_relays = calloc(relay_count,  sizeof(relay_t*));
    memcpy(tmp_relays, relays, relay_count * sizeof(relay_t*));
    //ESP_LOGW(TAG, "%s: increase relay_count", __func__);
    relay_count++;
    
    
    relays = realloc( relays, relay_count * sizeof(relay_t *));
    //ESP_LOGW(TAG, "%s: relays = %p", __func__, relays);
    
    memcpy(relays, tmp_relays, (relay_count-1) * sizeof(relay_t*));
    free(tmp_relays);

    relay_t* relay_p = (relay_t*) calloc(1, sizeof(relay_t));

    //ESP_LOGW(TAG, "%s: relay_p = %p", __func__, relay_p);
    //ESP_LOGW(TAG, "%s: relays[relay_count-1] = %p", __func__, relays[relay_count-1]);

    relay_p->pin = pin;
    relay_p->name = strdup(name);
    relay_p->level = level;
    relay_p->save = save;

    if ( relay_p->save ) {
        // get last state from nvs
        char s[8];
        sprintf(s, PARAM_RELAY_STATE, relay_p->pin);
        nvs_param_u8_load_def(PARAM_RELAYS, s, &relay_p->state, RELAY_STATE_OFF);
    } else {
        relay_p->state = RELAY_STATE_OFF;
    }


    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << pin;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    IOT_CHECK(TAG, gpio_config(&io_conf) == ESP_OK, NULL);

    //memcpy(&relays[ relay_count - 1], relay_p, sizeof(relay_t));
    relays[ relay_count - 1] = relay_p;

    //ESP_LOGI(TAG, "%s: created relay %s for pin %d", __func__, name, pin);

    // for (uint8_t i = 0; i < relay_count; i++)
    // {
    //     ESP_LOGW(TAG, "%s: relays[%d] = %p", __func__, i, relays[i]);
    // }

    //ESP_LOGW(TAG, "%s: relays[relay_count-1] = %p", __func__, relays[relay_count-1]);
    relay_write( relay_p, relay_p->state );
    return (relay_handle_t) relay_p;
}

esp_err_t relay_write(relay_handle_t relay_handle, relay_state_t state)
{
    relay_t* relay = (relay_t*) relay_handle;
    POINT_ASSERT(TAG, relay_handle);

    gpio_set_level(relay->pin, (0x01 & state) ^ relay->level);
    relay->state = state;

    if ( relay_status_queue != NULL )
    {
        xQueueSendToBack(relay_status_queue, relay, 0);
    }
    
    relay_save_state(relay);

    return ESP_OK;
}

relay_state_t relay_read(relay_handle_t relay_handle)
{
    relay_t* relay = (relay_t*) relay_handle;
    POINT_ASSERT(TAG, relay_handle);
    return relay->state;
}

esp_err_t relay_toggle(relay_handle_t relay_handle)
{
    relay_t* relay = (relay_t*) relay_handle;
    POINT_ASSERT(TAG, relay_handle);
    relay_state_t state = relay_read(relay);
    return relay_write(relay, !state);  
}

esp_err_t relay_delete(relay_handle_t relay_handle)
{
    POINT_ASSERT(TAG, relay_handle);
    relay_t *relay = (relay_t *) relay_handle;

    relay_t *r = (relay_t *) calloc( relay_count-1, sizeof(relay_t));
    uint8_t k = 0;
    for (uint8_t i = 0; i <relay_count; i++)
    {
        if (  ((relay_t *)relays[i])->pin != relay->pin )
        {
            memcpy(&r[k], (relay_t *)relays[i], sizeof(relay_t));
            k++;
        }
    }
    relay_count--;
    relays = (relay_t *)realloc(relays, relay_count * sizeof(relay_t *));
    memcpy(relays, r, relay_count * sizeof(relay_t));
    free(r);
    
    free(relay_handle);

    return ESP_OK;
}


void relay_load_nvs()
{
    #ifdef CONFIG_RELAY_CONFIG
    uint8_t cnt = 0;
    relay_t *_relays = NULL;

    if ( nvs_param_u8_load(PARAM_RELAYS, PARAM_RELAYS_COUNT,  &cnt) != ESP_OK ) {
        ESP_LOGE(TAG, "%s: unable load relay's count from nvs", __func__ );
        return;
    }    

    if ( cnt == 0 ) {
        ESP_LOGW(TAG, "%s: relays count = 0", __func__ );
        return;
    }

    // temporary array
    _relays = (relay_t * ) calloc( _relays, cnt * sizeof(relay_t));

    if ( nvs_param_load(PARAM_RELAYS, PARAM_RELAYS_DATA, _relays) != ESP_OK )
    {
        ESP_LOGE(TAG, "%s: unable load relay's data from nvs", __func__ );
        free( _relays );
        return;
    }

    // create relays
    for ( uint8_t i = 0; i < cnt; i++)
    {
        // relay_hrelay_read( (relay_handle_t)&relays[i]);
        relay_create( _relays[i].name, _relays[i].pin, _relays[i].level, _relays[i].save);
        if ( _relays[i].save ) {
            relay_write((relay_handle_t)relays[i],  _relays[i].state);
        }
    }

    free( _relays );
    #endif
}

void relay_save_nvs()
{
    #ifdef CONFIG_RELAY_CONFIG
    nvs_param_u8_save(PARAM_RELAYS, PARAM_RELAYS_COUNT, relay_count);
    nvs_param_save(PARAM_RELAYS, PARAM_RELAYS_DATA, relays, sizeof(relay_t) * relay_count);
    #endif
}
#endif