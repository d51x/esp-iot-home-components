#pragma once

#ifndef _MCP23017_MQTT_H_
#define _MCP23017_MQTT_H_

#include "mcp23017.h"
#include "mqtt_cl.h"

#define MCP23017_MQTT_SEND_TOPIC "mcp23017/gpio"

#ifdef CONFIG_MQTT_TOPIC_SEND_RECV
#define MCP23017_MQTT_RECV_TOPIC MCP23017_MQTT_SEND_TOPIC
#else
#define MCP23017_MQTT_RECV_TOPIC "mcp23017/set/gpio"
#endif

typedef struct mcp23017_mqtt {
    mcp23017_handle_t dev_h;
    uint8_t pin;
    #ifdef CONFIG_MQTT_TOPIC_SEND_RECV
    uint8_t prev;
    #endif
} mcp23017_mqtt_t;

void mcp23017_mqtt_periodic_send_cb(char *buf, void *args);
void mcp23017_mqtt_recv_cb(char *buf, void *args);
void mcp23017_mqtt_init(mcp23017_handle_t dev_h);

#endif