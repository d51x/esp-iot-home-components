


#pragma once

#ifndef _SHT21_H_
#define _SHT21_H_
#include "driver/i2c.h"
#include "nvsparam.h"
#include "i2c_bus.h"
#include "freertos/task.h"


#ifdef CONFIG_SENSOR_SHT21
#define SHT21_ADDR 0x40



typedef struct sht21 {
    float temp;
    float hum;
} sht21_t;



esp_err_t sht21_init();
void sht21_start(uint32_t delay); // seconds


void sht21_stop();
esp_err_t sht21_available();



// periodic task


// читает данные напрямую
float sht21_read_temp();
float sht21_read_hum();

// возвращает уже прочитанные данные структуры sht21
float sht21_get_temp();
float sht21_get_hum();

#endif //CONFIG_SENSOR_SHT21
#endif