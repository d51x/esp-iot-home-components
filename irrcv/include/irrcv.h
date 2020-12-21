#pragma once

#ifndef __IRRCV_H__
#define __IRRCV_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/ir_rx.h"
#include "nvsparam.h"

#ifdef CONFIG_COMPONENT_IR_RECV

#define IR_RX_BUF_LEN 128
#define IR_RECEIVE_DELAY CONFIG_IR_RECV_DELAY //100
#define IR_RECEIVE_GPIO CONFIG_IR_RECV_GPIO //4

// TSOP4838
typedef struct irrcv irrcv_t;
typedef void* irrcv_handle_t;

typedef struct ir_btn ir_btn_t;
typedef void (*button_cb)(void *user_ctx);
typedef void (*ir_rcv_cb)(void *arg);
typedef void (*handle_code_cb)(irrcv_handle_t ir_handle, uint32_t code);

struct ir_btn {
	uint32_t code;
	void *user_ctx;
	button_cb cb;
};


struct irrcv {
    uint8_t pin;
	uint16_t delay;
	uint8_t btn_cnt;
    uint32_t code;
	uint8_t enabled;
	ir_btn_t *btn;
	TaskHandle_t task; 
	ir_rcv_cb receive;
	handle_code_cb handle_code;
};

//irrcv_handle_t irrcv_init(uint8_t pin, uint16_t delay, uint8_t btn_cnt);
irrcv_handle_t irrcv_init();
void irrcv_deinit(irrcv_handle_t dev_h);
esp_err_t irrcv_add_button(irrcv_handle_t irrcv, uint8_t id, uint32_t code, void *user_ctx, button_cb cb);
void irrcv_start(irrcv_handle_t irrcv);
void irrcv_stop(irrcv_handle_t dev_h);
esp_err_t irrcv_restart(irrcv_handle_t dev_h);
void irrcv_load_cfg(irrcv_t *cfg);
void irrcv_save_cfg(const irrcv_t *cfg);


#endif 
#endif 