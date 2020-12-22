#pragma once

#ifndef _LEDCONTROL_MQTT_H_
#define _LEDCONTROL_MQTT_H_

#include "ledcontrol.h"
#include "mqtt_cl.h"

#ifdef CONFIG_LED_CONTROLLER

#define LEDCONTROL_MQTT_SEND_TOPIC "ledc/ch%d"

#ifdef CONFIG_MQTT_TOPIC_SEND_RECV
#define LEDCONTROL_MQTT_RECV_TOPIC LEDCONTROL_MQTT_SEND_TOPIC
#else
#define LEDCONTROL_MQTT_RECV_TOPIC "ledc/ch%d/set"
#endif

typedef struct ledcontrol_mqtt {
    ledcontrol_handle_t dev_h;
    uint8_t channel;
    #ifdef CONFIG_MQTT_TOPIC_SEND_RECV
    uint8_t prev;
    #endif
} ledcontrol_mqtt_t;

void ledcontrol_mqtt_periodic_send_cb(char **buf, void *args);
void ledcontrol_mqtt_recv_cb(char *buf, void *args);
void ledcontrol_mqtt_init(ledcontrol_handle_t dev_h);

#endif
#endif