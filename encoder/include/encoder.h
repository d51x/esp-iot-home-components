#ifndef __ENCODER_H__
#define __ENCODER_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "utils.h"


#define ENCODER_ROTATE_DEBOUNCE_TIME 50 // msec

typedef struct encoder encoder_t;
typedef struct encoder_config encoder_config_t;
typedef void* encoder_handle_t;

typedef void (*func_cb)(void *arg);

typedef enum {
	ENCODER_STATE_IDLE,
    ENCODER_STATE_PRESSED
} encoder_state_t;

typedef enum {
	ENCODER_DISABLED,
	ENCODER_ENABLED
} encoder_status_t;

typedef enum {
	ENCODER_ROTATE_ZERO,
	ENCODER_ROTATE_LEFT,    
    ENCODER_ROTATE_RIGHT	
} encoder_direction_t;

struct encoder_config {
    uint8_t pin_btn;
    uint8_t pin_clk;	
    uint8_t pin_dt;
	
	func_cb left;
	void *cb_left_ctx;
	
	func_cb right;
	void *cb_right_ctx;
	
	func_cb press;
	void *cb_press_ctx;

};

struct encoder {
    uint8_t pin_btn;
    uint8_t pin_clk;	
    uint8_t pin_dt;
	
	encoder_direction_t direction;
	encoder_state_t state;
	encoder_status_t status;
	int position;
	
	QueueHandle_t taskq;
	QueueHandle_t argq;
	TaskHandle_t task_rotate;

	func_cb left;
	void *cb_left_ctx;
	
	func_cb right;
	void *cb_right_ctx;
	
	func_cb press;
	void *cb_press_ctx;
	
	func_cb enable;
	func_cb disable;

	func_cb task_rotate_cb;	

	volatile long pause;  // Пауза для борьбы с дребезгом
	volatile long lastTurn;   // Переменная для хранения времени последнего изменения
	volatile int rotate_state; // Статус одного шага - от 0 до 4 в одну сторону, от 0 до -4 - в другую
	volatile int count;
};

encoder_handle_t encoder_init(encoder_config_t encoder_config);

#endif 