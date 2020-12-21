#include "esp_log.h"
#include "esp_attr.h"
#include "iot_debug.h"

static const char *TAG = "DBG";

#ifdef CONFIG_COMPONENT_DEBUG

#define DEBUG_LAST_STR_SIZE 32

static RTC_NOINIT_ATTR  char rtc_debug_str_last[DEBUG_LAST_STR_SIZE] = "";	
static RTC_NOINIT_ATTR  char rtc_debug_str_prev1[DEBUG_LAST_STR_SIZE] = "";	
static RTC_NOINIT_ATTR  char rtc_debug_str_prev2[DEBUG_LAST_STR_SIZE] = "";	
static RTC_NOINIT_ATTR  char rtc_debug_str_prev3[DEBUG_LAST_STR_SIZE] = "";	

static char _rtc_debug_str_last[DEBUG_LAST_STR_SIZE] = "";	
static char _rtc_debug_str_prev1[DEBUG_LAST_STR_SIZE] = "";	
static char _rtc_debug_str_prev2[DEBUG_LAST_STR_SIZE] = "";	
static char _rtc_debug_str_prev3[DEBUG_LAST_STR_SIZE] = "";	

#endif

#ifdef CONFIG_DEBUG_UART1
    #include "freertos/FreeRTOS.h"
    #include "driver/uart.h"

#define DEBUG_BUF_SIZE UART_FIFO_LEN + 1

void enable_debug_uart1()
{
	uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE		
    };
	
	uart_param_config(UART_NUM_1, &uart_config);
	uart_set_baudrate( UART_NUM_1, 115200);


	uint32_t br;
	uart_get_baudrate( UART_NUM_1, &br);
		#ifdef CONFIG_DEBUG_UART1
		userlog("%s: baudrate1 %d\n", __func__, br);
		#endif
		ESP_LOGW("DBG", "%s: baudrate1 %d\n", __func__, br);

	uart_driver_install(UART_NUM_1, DEBUG_BUF_SIZE, DEBUG_BUF_SIZE, 10, NULL, 0);	
	os_install_putc1(userlog);
}

void userlog(const char *fmt, ...) 
{
	char *str = (char *) malloc(100);
	memset(str, 0, 100);
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(str, 100, fmt, args);
    va_end(args);
	uart_write_bytes(UART_NUM_1, str, len);
	free(str);
	str = NULL;
}

#endif

#ifdef CONFIG_COMPONENT_DEBUG
void log_rtc_debug_str(const char *str)
{
	strncpy(rtc_debug_str_prev3, rtc_debug_str_prev2, DEBUG_LAST_STR_SIZE);
	strncpy(rtc_debug_str_prev2, rtc_debug_str_prev1, DEBUG_LAST_STR_SIZE);
	strncpy(rtc_debug_str_prev1, rtc_debug_str_last, DEBUG_LAST_STR_SIZE);
	strncpy(rtc_debug_str_last, str, DEBUG_LAST_STR_SIZE);
}

void log_rtc_print_debug_str()
{
	ESP_LOGE(TAG, "Prev3 debug str: %s", rtc_debug_str_prev3);
	ESP_LOGE(TAG, "Prev2 debug str: %s", rtc_debug_str_prev2);
	ESP_LOGE(TAG, "Prev1 debug str: %s", rtc_debug_str_prev1);
	ESP_LOGE(TAG, "Last debug str: %s", rtc_debug_str_last);
}

char *log_rtc_get_debug_str(uint8_t idx)
{
	static char *str = NULL;
	str = realloc(str, DEBUG_LAST_STR_SIZE);
	switch (idx) {
		case 0: strncpy( str, _rtc_debug_str_last, DEBUG_LAST_STR_SIZE); break;
		case 1: strncpy( str, _rtc_debug_str_prev1, DEBUG_LAST_STR_SIZE); break;
		case 2: strncpy( str, _rtc_debug_str_prev2, DEBUG_LAST_STR_SIZE); break;
		case 3: strncpy( str, _rtc_debug_str_prev3, DEBUG_LAST_STR_SIZE); break;
	}
	return str;
}

void log_rtc_init_debug_str()
{
	strncpy( _rtc_debug_str_last,  rtc_debug_str_last,  DEBUG_LAST_STR_SIZE);
	strncpy( _rtc_debug_str_prev1, rtc_debug_str_prev1, DEBUG_LAST_STR_SIZE);
	strncpy( _rtc_debug_str_prev2, rtc_debug_str_prev2, DEBUG_LAST_STR_SIZE);
	strncpy( _rtc_debug_str_prev3, rtc_debug_str_prev3, DEBUG_LAST_STR_SIZE);
}
#endif