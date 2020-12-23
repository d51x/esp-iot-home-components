#ifndef __DS18B20_H__
#define __DS18B20_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_system.h"
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"


#include "esp_log.h"


#include "onewire.h"

#define DSW_ADDR_PATTERN "%02x %02x %02x %02x %02x %02x %02x %02x"

typedef struct {
    float temp;
    uint8_t addr[8];
} ds18b20_t;


void ds18b20_init(uint8_t pin);
esp_err_t ds18b20_getTemp(const uint8_t *_addr, float *temp);

uint16_t ds18b20_get_crc_error_count();
//void ds18b20_search_task(void *arg);
//void ds18b20_get_temp_task(void *arg);
#endif /* __DSW_H__ */