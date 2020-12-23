#pragma once

#ifndef _SENSORS_H_
#define _SENSORS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
#include "utils.h"

 #define SENSOR_ERROR_ID -1
 //#define SENSORS_QUEUE_SIZE 10

typedef void (* func_sensors_print_cb)(char **payload, void *args); 

// typedef struct {
//     uint8_t id;
//     char *name; //[16]
// } sensor_group_t;

// typedef enum {
//     SENSOR_GPIO,   // локальные gpio
//     SENSOR_VGPIO,  // виртуальные gpio
//     SENSOR_DEFAULT,
//     SENSOR_TEMPERATURE,
//     SENSOR_HUMIDITY,
//     // SENSOR_PREASURE,
//     // SENSOR_LUMINOSITY,
//     // SENSOR_DISTANCE,
//     // SENSOR_ADC,
//     // SENSOR_VOLTAGE,
//     // SENSOR_CURRENT,
//     // SENSOR_POWER,
//     // SENSOR_ENERGY,
//     SENSOR_MAX
// } sensors_e;


// typedef union {
//     float f;
//     int32_t i32;
//     int16_t i16[2];
//     uint8_t b[4];
// } data_val_t;

typedef struct {
    int16_t id;         // -1 - empty sensor
    //sensors_e type;
    //uint8_t group_id;
    char *name; //[12]
    //char *title; //[16]
    //char *fmt; //[10]
    //char *dimen; //[4]
    //type_e val_type;
    //data_val_t value;
    // TODO: uint32_t update_dt;
    func_sensors_print_cb fn_cb;
    void *args;
} sensor_t;

// sensor_group_t *sensors_groups;
// uint8_t sensors_group_count;

sensor_t *sensors;
uint16_t sensors_count;

//QueueHandle_t sensors_queue;

//int8_t sensors_group_add(const char *name);
int16_t sensors_add(const char *name, func_sensors_print_cb cb, void *args);         // возвращает id датчика

void sensors_init();

// sensor_t *sensors_get_by_id(int16_t id);
// sensor_t *sensors_get_by_name(const char *name);

// data_val_t sensors_get_value_by_id(int16_t id);
// data_val_t sensors_get_value_by_name(const char *name);

//заполнять данные через Queue
// void sensors_set_value_by_id(uint16_t id, data_val_t value);
// void sensors_set_value_by_name(const char *name, data_val_t value);



#endif