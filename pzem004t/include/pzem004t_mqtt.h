#pragma once

#ifndef _PZEM004T_MQTT_H_
#define _PZEM004T_MQTT_H_

#include "pzem004t.h"
#include "mqtt_cl.h"

#ifdef CONFIG_SENSOR_PZEM004_T

void pzem_mqtt_init();

#endif
#endif