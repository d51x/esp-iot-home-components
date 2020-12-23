#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_system.h"
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"


#include "esp_log.h"




#define ONEWIRE_WRITE_SCRATCHPAD		0x4E
#define ONEWIRE_READ_SCRATCHPAD		0xBE
#define ONEWIRE_COPY_SCRATCHPAD 		0x48
#define ONEWIRE_READ_EEPROM 		0xB8
#define ONEWIRE_READ_PWRSUPPLY 		0xB4
#define ONEWIRE_SEARCHROM 		0xF0
#define ONEWIRE_SKIP_ROM			0xCC
#define ONEWIRE_READROM 			0x33
#define ONEWIRE_MATCHROM 		0x55
#define ONEWIRE_ALARMSEARCH 		0xEC
#define ONEWIRE_CONVERT_T		0x44



esp_err_t onewire_reset(uint8_t pin);


esp_err_t onewire_search(uint8_t pin, uint8_t *addr);
void onewire_select(uint8_t pin, const uint8_t rom[8]);
void onewire_skip(uint8_t pin);
void onewire_reset_search();

esp_err_t onewire_write(uint8_t pin, uint8_t v, int power);
//void onewire_write_bit(uint8_t v);
uint8_t onewire_read(uint8_t pin);
//uint8_t onewire_read_bit(void);
uint8_t onewire_crc8(const uint8_t *addr, uint8_t len);
uint16_t onewire_crc16(const uint16_t *data, const uint16_t  len);

#endif