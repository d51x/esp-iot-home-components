#ifndef _RELAY_H_
#define _RELAY_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"


#ifdef CONFIG_COMPONENT_RELAY

typedef void* relay_handle_t;

typedef enum {
    RELAY_LEVEL_HIGH = 0,
    RELAY_LEVEL_LOW = 1,    
} relay_level_t;

typedef enum {
    RELAY_STATE_OFF = 0,  // OFF
    RELAY_STATE_ON,     // ON
} relay_state_t;


typedef struct  {
	  gpio_num_t pin;
    relay_state_t state;
    relay_level_t level;
    relay_state_t prev;
    bool save;
    char *name;
} relay_t;   



extern relay_t **relays;
uint8_t relay_count;
extern QueueHandle_t relay_status_queue;

/**
  * @brief create relay object.
  *
  * @param io_num - pin 
  * @param level - on level - normal = HIGH, invert = LOW 
  *
  * @return relay_handle_t the handle of the relay created 
  */
relay_handle_t relay_create(const char *name, gpio_num_t io_num, relay_level_t level, bool save_state);

/**
  * @brief set state of relay
  *
  * @param  relay_handle
  * @param  state RELAY_STATUS_CLOSE or RELAY_STATUS_OPEN
  *
  * @return
  *     - ESP_OK: succeed
  *     - others: fail
  */
esp_err_t relay_write(relay_handle_t relay_handle, relay_state_t state);

/**
  * @brief get state of relay
  *
  * @param relay_handle
  *
  * @return state of the relay
  */
relay_state_t relay_read(relay_handle_t relay_handle);

/**
  * @brief free the memory of relay
  *
  * @param  relay_handle
  *
  * @return
  *     - ESP_OK: succeed
  *     - others: fail
  */
esp_err_t relay_delete(relay_handle_t relay_handle);

esp_err_t relay_toggle(relay_handle_t relay_handle);

void relay_save_nvs();
void relay_load_nvs();

#endif
#endif