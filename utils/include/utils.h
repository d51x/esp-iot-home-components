#pragma once

#ifndef __UTILS_H__
#define __UTILS_H__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <limits.h>
#include <errno.h>
#include "sys/time.h"
//#include <ctype.h>

#include "esp_log.h"
#include "esp_idf_version.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "driver/adc.h"
#include "nvs.h"
#include "esp_attr.h"
//#define FW_VER CONFIG_FW_VER //"1.4.3"
// Application version info  "1.19.17.10(861fe60)"
extern char FW_VER[32];


#define IOT_CHECK(tag, a, ret)  if(!(a)) {                                             \
        ESP_LOGE(tag,"%s:%d (%s)", __FILE__, __LINE__, __FUNCTION__);      \
        return (ret);                                                                   \
        }
#define ERR_ASSERT(tag, param)  IOT_CHECK(tag, (param) == ESP_OK, ESP_FAIL)
#define POINT_ASSERT(tag, param, ret)    IOT_CHECK(tag, (param) != NULL, (ret))

#define LOG_FMT(x)      "%s: " x, __func__

#define B(bit_no)         (1 << (bit_no))
#define BIT_CLEAR(reg, bit_no)   (reg) &= ~B(bit_no)
#define BIT_SET(reg, bit_no)   (reg) |= B(bit_no)
#define BIT_CHECK(reg, bit_no)   ( (reg) & B(bit_no) )
#define BIT_TRIGGER(reg, bit_no)   (reg) ^= B(bit_no)

#define micros() (unsigned long) (esp_timer_get_time())
#define millis() (unsigned long) (esp_timer_get_time() / 1000ULL)
#define cur_sec() (uint32_t) (esp_timer_get_time() / 1000ULL / 1000U)
#define IP_2_STR(a) ip4addr_ntoa(a)

#define pauseTask(delay)  (vTaskDelay(delay / portTICK_PERIOD_MS))

#define TM_YEAR_BASE 1900

#define UPTIME2STR "%d days %02dh %02dm %02ds"
#define UPTIMESTRLENMAX 20

#define LOCALTIME2STR "%a, %d.%m.%Y %X"  //http://all-ht.ru/inf/prog/c/func/strftime.html
#define LOCALTIMESTRLENMAX 30

#define MAP_255_TO_100(x) (x * 100 / 255 )
#define MAP_100_TO_255(x) (x * 255 / 100 )
#define GET_MIN(x, y)   ( ( x < y ) ? x : y )
#define GET_MAX(x, y)   ( ( x > y ) ? x : y )

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c" 
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

  // printf("Leading text "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(byte));
  // For multi-byte types
  //  printf("m: "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN"\n",
  //              BYTE_TO_BINARY(m>>8), BYTE_TO_BINARY(m));
/*
typedef enum {
    STR2INT_SUCCESS,        // 0
    STR2INT_OVERFLOW,       // 1
    STR2INT_UNDERFLOW,      // 2
    STR2INT_INCONVERTIBLE   // 3
} str2int_errno;
*/

typedef enum {
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_INT8,
    TYPE_INT16,
    TYPE_INT32,
    TYPE_FLOAT,
    TYPE_STRING
} type_e;

typedef struct {
    uint32_t chip_id;
    esp_chip_model_t chip_model;
    uint8_t chip_revision;
    uint32_t features;
} m_chip_info_t;

typedef struct {
    uint32_t flash_size;
    uint32_t free_heap_size;
    uint32_t flash_size_map;    
} m_mem_info_t;

typedef struct {
    wifi_mode_t wifi_mode;
    uint32_t wifi_reconnect;
    ip4_addr_t ip;
    //uint8_t ip[4];
    uint8_t mac[6];
    uint8_t status;
} m_wifi_info_t;

typedef struct {
    m_chip_info_t chip_info;
    m_mem_info_t mem_info;
    char sdk_version[30];
    uint32_t vdd33;
} system_info_t;

extern const char *RESET_REASONS[ESP_RST_SDIO+1];

m_wifi_info_t wifi_info;

uint8_t str_to_int(int *out, char *s, int base);
uint8_t str_to_long(long *out, char *s, int base);
uint8_t str_to_uint16(uint16_t *out, char *s, int base);
uint8_t str_to_uint8(uint8_t *out, char *s, int base);

uint16_t get_adc();
void print_chip_info();
void get_system_info(system_info_t *sys_info);
char* print_wifi_mode(wifi_mode_t mode);

uint32_t get_chip_id(uint8_t *mac);


void get_uptime(char*  buf);
void get_localtime(char*  buf);

void get_timeinfo(struct tm *_timeinfo);
uint32_t get_time(char* f);

void trim(char *s);

void systemRebootTask(void *arg);
int url_decode(const char *s, char *dec);

long map(long x, long in_min, long in_max, long out_min, long out_max);
uint32_t hex2int(char *hex);

uint32_t uround(float);


void print_task_stack_depth(const char *TAG, const char *task_name);

#ifdef CONFIG_DEBUG_PRINT_TASK_INFO

void print_tasks_info();
#endif

char* cut_str_from_str(char *str, const char *str2);
char* copy_str_from_str(const char *str, const char *str2);

int get_buf_size(const char* format, ...);


#endif /* __UTILS_H__ */