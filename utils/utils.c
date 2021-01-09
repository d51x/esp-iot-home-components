#include "utils.h"



static const char *TAG = "utils";

char FW_VER[32] = "";

const char *RESET_REASONS[ESP_RST_SDIO+1] = {
    "undetermined",
    "power-on event",
    "by external pin",
    "esp_restart",
    "exception/panic",
    "interrupt watchdog",
    "task watchdog",
    "other watchdogs",
    "after deep sleep",
    "Brownout reset",
    "Reset over SDIO",
};

uint16_t get_adc() {
    uint16_t adc = 0;
    adc_config_t adc_cfg;
    adc_cfg.mode = ADC_READ_TOUT_MODE;
    adc_cfg.clk_div = 8;
    if ( adc_init(&adc_cfg) == ESP_OK ) {
        if ( adc_read(&adc) == ESP_OK ) {
        } else {
            ESP_LOGE(TAG, "FAIL: adc_read error\n"); 
        }
        adc_deinit();
    } else {
       ESP_LOGE(TAG, "FAIL: adc_init error\n"); 
    }
    return adc;
}

uint32_t get_chip_id(uint8_t *mac){
 return mac[5] + mac[4]*256 + mac[3]*256*256;
}

void get_system_info(system_info_t *sys_info) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    m_chip_info_t m_chip_info;
    m_chip_info.chip_id = get_chip_id(wifi_info.mac);
    //m_chip_info.chip_id = get_chip_id(wifi_get_mac());
    m_chip_info.chip_model = chip_info.model;
    m_chip_info.chip_revision = chip_info.revision;
    m_chip_info.features = chip_info.features;
    memcpy(&sys_info->chip_info, &m_chip_info, sizeof(m_chip_info_t)); 

    m_mem_info_t m_mem_info;
    m_mem_info.flash_size = spi_flash_get_chip_size();
    m_mem_info.free_heap_size = esp_get_free_heap_size();
    m_mem_info.flash_size_map = system_get_flash_size_map();
    memcpy(&sys_info->mem_info, &m_mem_info, sizeof(m_mem_info_t)); 

//  Free Mem - sendContentBlocking
//  Free Stack - 3623 (2320 - sendWebPage)
//  local time
// uptime from millis to day hour minutes seconds
// rssi

    //char *sdk_version = malloc(20);
    //sdk_version = esp_get_idf_version();
    memset(&sys_info->sdk_version, 0, 30);
    memcpy(&sys_info->sdk_version, esp_get_idf_version(), 30);
    //free(sdk_version);

    uint32_t vdd33 = esp_wifi_get_vdd33();
    memcpy(&sys_info->vdd33, &vdd33, sizeof(uint32_t));
    
}

char* print_wifi_mode(wifi_mode_t mode){
    switch (mode) {
        case WIFI_MODE_STA:
            return "STA";
            break;
        case WIFI_MODE_AP:
            return "AP";
            break;
        case WIFI_MODE_APSTA:
            return "STA+AP";
            break;
        default:
            return "";
            break;
    }
}

void print_chip_info() {
    system_info_t *sys_info = malloc(sizeof(system_info_t));
    get_system_info(sys_info);

    printf("CHIP INFO\n");
    printf("Model: %s\n", (sys_info->chip_info.chip_model == 0) ? "esp8266" : "esp32");
    printf("rev. %d\n", sys_info->chip_info.chip_revision);
    
    printf("IDF version: %s\n", sys_info->sdk_version);

    printf("Flash size: %d Mb\n", sys_info->mem_info.flash_size / (1024 * 1024));
    printf("Free heap size: %d\n", sys_info->mem_info.free_heap_size);
    printf("Flash size map: %d\n", sys_info->mem_info.flash_size_map);

    printf("VCC: %d\n", sys_info->vdd33);

    printf("Wifi reconnect: %d\n", wifi_info.wifi_reconnect);
    printf("Wifi status: %s\n", (wifi_info.status) ? "connected" : "disconnected");
    printf("Wifi mode: %s\n", print_wifi_mode(wifi_info.wifi_mode));
    printf("IP address: %s\n", IP_2_STR(&wifi_info.ip));
    printf("MAC address: "MACSTR"\n", MAC2STR(wifi_info.mac));
    
    free(sys_info);
        
            //uint32 system_get_userbin_addr(void);

        // OTA
        // uint8 system_upgrade_userbin_check(void); // 0x00 : UPGRADE_FW_BIN1, i.e. user1.bin
        // 0x01 : UPGRADE_FW_BIN2, i.e. user2.bin
        // rssi
        // spiffs info
    
}


void get_uptime(char* buf){
    //char* buf = malloc(20);
    time_t now;
    struct tm timeinfo;
    now = millis()/1000UL;
    setenv("TZ", "UTC-0", 1);
    tzset();    
    localtime_r(&now, &timeinfo);
    snprintf(buf, UPTIMESTRLENMAX, UPTIME2STR,  timeinfo.tm_yday,
                                                        timeinfo.tm_hour,
                                                        timeinfo.tm_min,
                                                        timeinfo.tm_sec);
   //return buf;
}

void get_localtime(char* buf){
    //char *buf = malloc(LOCALTIMESTRLENMAX);
    time_t now;
    struct tm timeinfo;
    setenv("TZ", "UTC-3", 1);
    tzset();    
    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(buf, LOCALTIMESTRLENMAX, LOCALTIME2STR, &timeinfo);
/*
    snprintf(buf, LOCALTIMESTRLENMAX, LOCALTIME2STR,  
                                                        timeinfo.tm_wday,
                                                        timeinfo.tm_mday,
                                                        timeinfo.tm_mon,
                                                        timeinfo.tm_year,
                                                        timeinfo.tm_hour,
                                                        timeinfo.tm_min,
                                                        timeinfo.tm_sec);
*/
    //return buf;
}

void get_timeinfo(struct tm *_timeinfo)
{
    time_t now;
    setenv("TZ", "UTC-3", 1);
    tzset();    
    time(&now);
    localtime_r(&now, _timeinfo);
}

uint32_t get_time(char* f){
    time_t now;
    struct tm timeinfo;
    setenv("TZ", "UTC-3", 1);
    tzset();    
    time(&now);
    localtime_r(&now, &timeinfo);

    if ( strcmp(f, "hour") == ESP_OK ) 
    {
        return timeinfo.tm_hour;
    } 
    else if ( strcmp(f, "min") == ESP_OK ) 
    {
        return timeinfo.tm_min;
    } 
    else if ( strcmp(f, "sec") == ESP_OK ) 
    {
        return timeinfo.tm_sec;
    } 
    else if ( strcmp(f, "min_of_day") == ESP_OK ) 
    {
        return timeinfo.tm_hour * 60 + timeinfo.tm_min;
    }
    else if ( strcmp(f, "sec_of_day") == ESP_OK ) 
    {
        return timeinfo.tm_hour * 3660 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
    }
    else 
    {
        return now;
    }
}

    
uint8_t str_to_long(long *out, char *s, int base) {
    char *end;
    //if (s[0] == '\0' || isspace(s[0]))
    //    return STR2INT_INCONVERTIBLE;
    errno = 0;
    long l = strtol(s, &end, base);
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    if (l > LONG_MAX || (errno == ERANGE && l == LONG_MAX))
        return 1;
    if (l < LONG_MIN || (errno == ERANGE && l == LONG_MIN))
        return 2;
    if (*end != '\0')
        return 3;
    *out = l;
    return 0;
}

uint8_t str_to_int(int *out, char *s, int base) {
    char *end;
    //if (s[0] == '\0' || isspace(s[0]))
    //    return STR2INT_INCONVERTIBLE;
    errno = 0;
    long l = strtol(s, &end, base);
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
        return 1;
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
        return 2;
    if (*end != '\0')
        return 3;
    *out = l;
    return 0;
}

uint8_t str_to_uint16(uint16_t *out, char *s, int base) {
    char *end;
    //if (s[0] == '\0' || isspace(s[0]))
    //    return STR2INT_INCONVERTIBLE;
    errno = 0;
    unsigned long l = strtoul(s, &end, base);
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    if (l > UINT_MAX || (errno == ERANGE && l == ULONG_MAX))
        return 1;
    if (*end != '\0')
        return 3;
    *out = l;
    return 0;
}

uint8_t str_to_uint8(uint8_t *out, char *s, int base) {
    char *end;
    //if (s[0] == '\0' || isspace(s[0]))
    //    return STR2INT_INCONVERTIBLE;
    errno = 0;
    unsigned long l = strtoul(s, &end, base);
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    if (l > UCHAR_MAX || (errno == ERANGE && l == UCHAR_MAX))
        return 1;
    if (*end != '\0')
        return 3;
    *out = l;
    return 0;
}

void trim(char *s){
    // удаляем пробелы и табы с начала строки:
    int i = 0, j;
    while ( (s[i] == ' ') || (s[i] == '\t') ) i++;
    if ( i > 0 ) {
        for ( j = 0; j < strlen(s); j++) s[j] = s[j+i];
        s[j] = '\0';
    }
 
    // удаляем пробелы и табы с конца строки:
    i = strlen(s) - 1;
    while ( (s[i]==' ') || (s[i]=='\t') ) i--;
    if ( i < (strlen(s) - 1 ) ) s[i+1]='\0';
}

void rtrim( char * string, char * trim )
{
    // делаем обрезку справа
    for( int i = strlen (string) - 1; i >= 0 && strchr (trim, string[i]) != NULL; i-- )
    {  
        // переставляем терминатор строки 
        string[i] = '\0';
    }
}
 
void ltrim( char * string, char * trim )
{
    // делаем обрезку слева
    while ( string[0] != '\0' && strchr ( trim, string[0] ) != NULL )
    {
        memmove( &string[0], &string[1], strlen(string) );
    }
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void systemRebootTask(void *arg)
{
    ESP_LOGE("OTA", "Reboot Command, Restarting within %d", (uint32_t)arg);
	vTaskDelay((uint32_t)arg / portTICK_PERIOD_MS);
    esp_restart();
}




inline int ishex(int x)
{
	return	(x >= '0' && x <= '9')	||
		(x >= 'a' && x <= 'f')	||
		(x >= 'A' && x <= 'F');
}
	
int url_decode(const char *s, char *dec)
{
	char *o;
	const char *end = s + strlen(s);
	int c;
 
	for (o = dec; s <= end; o++) {
		c = *s++;
		if (c == '+') c = ' ';
		else if (c == '%' && (	!ishex(*s++)	||
					!ishex(*s++)	||
					!sscanf(s - 2, "%2x", &c)))
			return -1;
 
		if (dec) *o = c;
	}
 
	return o - dec;
}

uint32_t hex2int(const char *hex) {
    uint32_t val = 0;
    while (*hex) {
        // get current character then increment
        char byte = *hex++; 
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
        // shift 4 to make space for new digit, and add the 4 bits of the new digit 
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}

uint32_t uround(float val) {
    uint32_t val2 = val * 10;  // 238,9 * 10 = 2389
    uint32_t res = val2 / 10; // 238
    if ( val2 % 10 >=5 ) ++res;
    return res;
}

void print_task_stack_depth(const char *TAG, const char *func_name){
    #if CONFIG_FREERTOS_USE_TRACE_FACILITY  && CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
    TaskStatus_t xTaskDetails;
    vTaskGetInfo( NULL, &xTaskDetails, pdTRUE, eInvalid );
    ESP_LOGI(TAG, "Function: %s, Task %s stack depth: %d", func_name, xTaskDetails.pcTaskName, xTaskDetails.usStackHighWaterMark);

    //UBaseType_t uxHighWaterMark;
    //uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    //ESP_LOGI(TAG, "%s stack depth: %d", task_name, uxHighWaterMark);
    #endif
}

#ifdef CONFIG_DEBUG_PRINT_TASK_INFO
char get_task_state_name(eTaskState _t) 
{
    char res;
    
    if ( _t  == eRunning )      res = 'A';
    else if ( _t == eReady )    res = 'R';
    else if ( _t == eBlocked )  res = 'B';
    else if ( _t == eSuspended) res = 'S';
    else if ( _t == eDeleted)   res = 'D'; 
    else res = 'U';
    return res;
}

void print_tasks_info()
{
    UBaseType_t tasks_count = uxTaskGetNumberOfTasks();
    ESP_LOGI(TAG, "------------------------------------");
    //ESP_LOGI(TAG, "----- Tasks: %d", tasks_count);
    ESP_LOGI(TAG, "----- Heap free size: %d", esp_get_free_heap_size());
    //ESP_LOGI(TAG, "Minimal Heap free size: %d", xPortGetMinimumEverFreeHeapSize());

    unsigned long _total_runtime, ulStatsAsPercentage;
    TaskStatus_t *tasks_info = (TaskStatus_t *) calloc(tasks_count, sizeof(TaskStatus_t));
    if ( tasks_info != NULL ) 
    {
       // configGENERATE_RUN_TIME_STATS 
       tasks_count = uxTaskGetSystemState(tasks_info, tasks_count, &_total_runtime);
       ESP_LOGI(TAG, "----- Tasks: %d", tasks_count);

       _total_runtime /= 100UL; //percents
        ESP_LOGI(TAG, "----- total runtime: %lu", _total_runtime);

        //if ( _total_runtime > 0 )
        //{
           for ( uint8_t i = 0; i < tasks_count; i++)
            {
                
                ulStatsAsPercentage = ( _total_runtime > 0 ) ?  tasks_info[ i ].ulRunTimeCounter / _total_runtime : 0;
                ESP_LOGI(TAG, "---- %02d Task name: %20s, state: %c, priority: %2d (%2d), free stack: %5d, run time: %10d\t\t, percent: %lu%%", 
                            tasks_info[ i ].xTaskNumber,
                            tasks_info[ i ].pcTaskName,
                            get_task_state_name( tasks_info[ i ].eCurrentState ),
                            tasks_info[ i ].uxCurrentPriority,
                            tasks_info[ i ].uxBasePriority,
                            tasks_info[ i ].usStackHighWaterMark,
                            tasks_info[ i ].ulRunTimeCounter,
                            ulStatsAsPercentage);
                            

           }
        //}

    }
    ESP_LOGI(TAG, "------------------------------------\n");
    free(tasks_info);
}
#endif

char* cut_str_from_str(char *str, const char *str2)
{
    if ( strlen(str) == 0 ) return NULL;
    char *p = strstr(str, str2);
    int pos = 0;
    if ( p != NULL ) {
        pos = p - str;
    } else {
        pos = strlen(str);
    }

    char *r = (char *) calloc(1, pos + 1);
    strncpy(r, str, pos);

    if ( p != NULL ) {
        strcpy(str, p+1);
    } else {
         memset(str, 0, strlen(str)+1);
    }
    
    return r;
}

char* copy_str_from_str(const char *str, const char *str2)
{
    char *p = strstr(str, str2);
    uint8_t pos = p - str;
    p = (char *) calloc(1, pos + 1);
    strncpy(p, str, pos);
    return p;
}

int get_buf_size(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buf[100];
    int result = vsnprintf(buf, 100, format, args);
    va_end(args);
    return result + 1; // safe byte for \0
}
