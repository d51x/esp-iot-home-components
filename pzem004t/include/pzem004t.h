#ifndef __PZEM004T_H__
#define __PZEM004T_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"


#ifdef CONFIG_SENSOR_PZEM004_T_SOFTUART
#include "softuart.h"
#else 
#include "driver/uart.h"
#endif

#include "nvsparam.h"

#ifdef CONFIG_DEBUG_UART1
#include "iot_debug.h"
#endif

#ifdef CONFIG_SENSOR_PZEM004_T

#define PZEM_FLOAT_DIVIDER 1000.0f

#define PZEM_ERROR_VALUE 0 //-1.0f

typedef uint8_t PZEM_Address[4] ;

#ifdef CONFIG_SENSOR_PZEM004_T_CALC_CONSUMPTION
	#define PZEM_ENERGY_ZONE_T1_HOUR CONFIG_SENSOR_PZEM004_T_CONSUMPTION_T1 //7
	#define PZEM_ENERGY_ZONE_T2_HOUR CONFIG_SENSOR_PZEM004_T_CONSUMPTION_T2 //23
    typedef struct {
        uint32_t prev_midnight;
        uint32_t today_midnight;
        uint32_t prev_t1;
        uint32_t prev_t2;
        uint32_t today_t1;
        uint32_t today_t2;
    } pzem_energy_t;

    typedef struct {
        uint16_t today_total;
        uint16_t prev_total;
        uint16_t today_day;
        uint16_t prev_day;
        uint16_t today_night;
        uint16_t prev_night;
    } pzem_consumption_t;
#endif

typedef struct {
    float voltage;
    float current;
    float power;
    float energy;
    uint16_t errors;
    esp_err_t ready;
    #ifdef CONFIG_SENSOR_PZEM004_T_CALC_CONSUMPTION
        pzem_energy_t energy_values;
        pzem_consumption_t consumption;
    #endif
} pzem_data_t;

typedef struct {
    uint8_t voltage_read_count; // кол-во чтений за 1 раз
    uint8_t current_read_count;
    uint8_t power_read_count;
    uint8_t energy_read_count;
} pzem_read_strategy_t;

void pzem_init(uint8_t uart_num);
esp_err_t pzem_set_addr(PZEM_Address *addr);

// запускает задачу на чтение параметров
void pzem_task_start(uint32_t delay_sec);

void pzem_set_read_strategy(pzem_read_strategy_t strategy);

// функции чтения свежих данных из устройства
float pzem_read_voltage();
float pzem_read_current();
float pzem_read_power();
float pzem_read_energy();

// функции получения уже прочитанных данных из устройства
pzem_data_t pzem_get_data();

// обнулить счетчики потребления ээ за вчера и сегодня (today = true)
void pzem_reset_consumption(bool today);

#endif

#endif 