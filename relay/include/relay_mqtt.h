#pragma once

#ifndef _RELAY_MQTT_H_
#define _RELAY_MQTTrelay_H_

#include "relay.h"
#include "mqtt_cl.h"

#ifdef CONFIG_COMPONENT_RELAY

#define RELAY_MQTT_SEND_TOPIC "relay%d"

#ifdef CONFIG_MQTT_TOPIC_SEND_RECV
    #define RELAY_MQTT_RECV_TOPIC RELAY_MQTT_SEND_TOPIC
#else
    #define RELAY_MQTT_RECV_TOPIC "relay%d/set"
#endif

typedef struct relay_mqtt {
    relay_handle_t dev_h;
    uint8_t pin;
    
    #ifdef CONFIG_MQTT_TOPIC_SEND_RECV
        uint8_t prev;
    #endif
} relay_mqtt_t;

void relay_mqtt_init();

#endif
#endif