#pragma once

#ifndef _RGBCONTROL_MQTT_H_
#define _RGBCONTROL_MQTT_H_

#include "rgbcontrol.h"
#include "mqtt_cl.h"



#ifdef CONFIG_RGB_CONTROLLER

typedef struct rgbcontrol_mqtt {
    rgbcontrol_handle_t dev_h;
    #ifdef CONFIG_MQTT_TOPIC_SEND_RECV
    uint8_t prev;
    #endif
} rgbcontrol_mqtt_t;

void rgbcontrol_mqtt_periodic_send_cb(char *buf, void *args);
void rgbcontrol_mqtt_recv_cb(char *buf, void *args);
void rgbcontrol_mqtt_init(rgbcontrol_handle_t dev_h);

#endif
#endif