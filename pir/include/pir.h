#ifndef __PIR_H__
#define __PIR_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"


typedef struct pir pir_t;
typedef struct pir_conf pir_conf_t;
typedef void* pir_handle_t;

typedef void (*func_cb)(void *arg);
typedef void (*func_interval_cb)(void *arg, uint16_t interval);

typedef enum {
    PIR_LEVEL_DISABLE,
	PIR_LEVEL_LOW,    
    PIR_LEVEL_HIGH,   
	PIR_LEVEL_ANY
} pir_active_level_t;

typedef enum {
	PIR_ISR,
	PIR_POLL
} pir_read_t;

typedef enum {
	PIR_DISABLED,
	PIR_ENABLED
} pir_status_t;

struct pir_conf {
    uint8_t pin;
	pir_active_level_t active_level;
	pir_read_t type;
	
	void *cb_high_ctx;
	func_cb high_cb;			// callback for start interrupt
	
	void *cb_low_ctx;
	func_cb low_cb;

	int poll_interval;
	int interval_low;
	void *cb_tmr_low_ctx;
	func_cb tmr_low_cb;		// callback for end timer after start interrupt

	int interval_high;
	void *cb_tmr_high_ctx;
	func_cb tmr_high_cb;		// callback for end timer after start interrupt


};

struct pir {
    uint8_t pin;
	pir_status_t status;
	pir_active_level_t active_level;
	pir_read_t type;
	
	void *cb_high_ctx;
	func_cb high_cb;			// callback for start interrupt
	
	void *cb_low_ctx;
	func_cb low_cb;			// callback for start interrupt
	
	QueueHandle_t taskq;
	QueueHandle_t argq;
	TaskHandle_t task;
	TaskHandle_t task_poll;

	int poll_interval;

	TimerHandle_t tmr_low;
	TimerHandle_t tmr_high;
	func_cb task_cb;


	TickType_t interval_low;
	TickType_t interval_high;
	void *cb_tmr_low_ctx;
	func_cb tmr_low_cb;		// callback for end timer after start interrupt
	void *cb_tmr_high_ctx;
	func_cb tmr_high_cb;		// callback for end timer after start interrupt

	func_cb enable;
	func_cb disable;	
	func_interval_cb set_interval_low;	
	func_interval_cb set_interval_high;	
};

pir_handle_t pir_init(pir_conf_t pir_conf);


#endif 