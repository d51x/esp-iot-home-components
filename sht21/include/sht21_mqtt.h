#pragma once

#ifndef _SHT21_MQTT_H_
#define _SHT21_MQTT_H_

#include "sht21.h"
#include "mqtt_cl.h"

#ifdef CONFIG_SENSOR_SHT21

#define SHT21_MQTT_SEND_TOPIC_TEMP "sht21/temp"
#define SHT21_MQTT_SEND_TOPIC_HUM "sht21/hum"

void sht21_mqtt_init();

#endif
#endif