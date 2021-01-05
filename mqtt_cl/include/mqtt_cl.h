/*
* TODO:
*       - задавать название для каждого утвзщште через веб
*           endpoints: "endp1:name1;endp2:name"
*       - хранить datetime последнего получения данных для endpoint
*       - выводить рядом со значением datetime последнего получения данных
*/

#pragma once

#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "mqtt_client.h" 
#include "nvsparam.h"
#include "wifi.h"
#include "utils.h"


#define MQTT_BROKER_URL CONFIG_MQTT_URL //"mqtt://192.168.2.63:1883"
#define MQTT_SEND_INTERVAL 60  // sec

#define MQTT_PAYLOAD_SIZE_DEFAULT 10
#define MQTT_CLIENT_ID_MASK "ESP_%02X%02X%02X"

#define MQTT_DEVICE_UPTIME          "uptime"
#define MQTT_DEVICE_FREEMEM         "freemem"
#define MQTT_DEVICE_RSSI            "rssi"

#define MQTT_CFG_LOGIN_LENGTH 16

typedef struct {
    char broker_url[32];
    char login[MQTT_CFG_LOGIN_LENGTH];
    char password[16];
    char base_topic[16];
    uint16_t send_interval;
    uint8_t enabled;
} mqtt_config_t;



uint32_t mqtt_error_count;
uint32_t mqtt_reconnects;
uint8_t mqtt_state;

typedef void (* func_mqtt_send_cb)(char **payload, void *args);  
typedef void (* func_mqtt_recv_cb)(const char *payload, void *args);  
// typedef void (*func_mqtt_send_cb)(const char *topic, const char *payload);

#define TOPIC_END_NAME_LENGTH 20

typedef struct {
    char topic[TOPIC_END_NAME_LENGTH];
    func_mqtt_send_cb fn_cb;
    void *args;
} mqtt_send_t;
#define MQTT_SEND_CB 5

typedef struct {
    //char topic[TOPIC_END_NAME_LENGTH];
    char *topic;
    uint8_t inner;
    func_mqtt_recv_cb fn_cb;
    void *args;
} mqtt_recv_t;
#define MQTT_RECV_CB 5

void mqtt_init(); /*const char *broker_url, uint16_t send_interval*/
void mqtt_start();
void mqtt_stop();
void mqtt_restart_task();

void mqtt_load_cfg(mqtt_config_t *cfg);
void mqtt_get_cfg(mqtt_config_t *cfg);
void mqtt_save_cfg(const mqtt_config_t *cfg);
void mqtt_set_device_name(const char *dev_name);
void mqtt_publish(const char *_topic, const char *payload);
void mqtt_publish_external(const char *_topic, const char *payload);
void mqtt_unsubscribe_topic(const char *topic);
void mqtt_subscribe_topic(const char *topic);
// просто отправить данные
// mqtt publish - topic w/o device_name and login + data



// ФУНКЦИИ БЕЗ ПРОВЕРОК И ДИНАМИЧЕСКОГО ВЫДЕЛЕНИЯ ПАМЯТИ
// НУЖНО ИЗМЕНИТЬ ЗНАЧЕНИЯ MQTT_SEND_CB и MQTT_RECV_CB, если мало
// название топика не длинее TOPIC_END_NAME_LENGTH - название без учета  "login/hostname/""
// зарегистрировать функцию колбека, которая будет вызвана при периодической отправки данных с настроенным интервалом
void mqtt_add_periodic_publish_callback( const char *topic, func_mqtt_send_cb fn_cb, void *args);
void mqtt_del_periodic_publish_callback( const char *topic, func_mqtt_send_cb fn_cb, void *args);

// зарегистрировать функцию колбека, которая будет вызвана при получении данных в указанном топике
// topic должен содержать часть пути после  "login/hostname"

/**
  * @brief  зарегистрировать функцию колбека, которая будет вызвана при получении данных в указанном топике
  *
  * @param  topic topic path, length 64 symbols
  * @param  inner_topic  Внутренний топик, содержить путь после "login/hostname" 
  * @param  fn_cb  callback, который будет вызван при получении данных в топик 
  *
  */
void mqtt_add_receive_callback( const char *topic, uint8_t inner_topic, func_mqtt_recv_cb fn_cb, void *args);  
void mqtt_del_receive_callback( const char *topic, uint8_t inner_topic, func_mqtt_recv_cb fn_cb, void *args);  

