#ifndef __WIFI_H__
#define __WIFI_H__

//#include <stdio.h>
#include <string.h>
//#include <stdlib.h>

#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_log.h"
#include "nvsparam.h"

#define ESP_WIFI_SSID      "" //"DmintyIot"
#define ESP_WIFI_PASS      "12346578"

#define ESP_WIFI_AP_SSID   "TestIot"
#define ESP_WIFI_AP_PASS    "1234"
#define ESP_WIFI_AP_IP_ADDR_1 0
#define ESP_WIFI_AP_IP_ADDR_2 168
#define ESP_WIFI_AP_IP_ADDR_3 5
#define ESP_WIFI_AP_IP_ADDR_4 1

#define MAX_STA_CONN 4

#define ESP_WIFI_HOSTNAME   ""   
#define ESP_MAXIMUM_RETRY  5

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

typedef struct {
    char ssid[16];          // sdk: 32
    char password[16];      // sdk: 64
    wifi_mode_t mode;
    char hostname[TCPIP_HOSTNAME_MAX_SIZE];
    uint8_t first;
    char ip[16];
} wifi_cfg_t;

wifi_cfg_t *wifi_cfg;

EventGroupHandle_t xWiFiEventGroup;

void wifi_init();
void wifi_deinit();

void wifi_init_sta(void);
void wifi_init_ap(void);

int8_t wifi_get_rssi();
void wifi_get_mac(char *mac);

bool isWiFiConnected();

void wifi_cfg_load(wifi_cfg_t *cfg);
void wifi_cfg_save(const wifi_cfg_t *cfg);

uint32_t wifi_get_reconnect_count();

#endif /* __WIFI_H__ */