
#include "sntp.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "lwip/apps/sntp.h"
#include "wifi.h"
#include "utils.h"

#define NTP_TASK_DELAY 5*1000*60  // 5 min
#define NTP_TASK_TIMEOUT 5000  //msec
#define NTP_TASK_PRIORITY 13

//#ifdef DEBUG
static const char *TAG = "SNTP";
//#endif

void sntp_task(void *arg);
void obtain_time(void);

void vSntpTaskCb(void *arg){

    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);
    
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        obtain_time();
    }

    setenv("TZ", "UTC-3", 1);
    tzset();

    while (1) {
        // update 'now' variable with current time
        time(&now);
        localtime_r(&now, &timeinfo);
        vTaskDelay(NTP_TASK_DELAY / portTICK_RATE_MS);
    }
}

void obtain_time(void)
{
    xEventGroupWaitBits(xWiFiEventGroup, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    //xEventGroupWaitBits(ota_event_group, OTA_IDLE_BIT, false, true, portMAX_DELAY);
//    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;

    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        vTaskDelay(NTP_TASK_TIMEOUT / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

void sntp_start(){
    // from sntp example: SNTP service uses LwIP, please allocate large stack space.
    xTaskCreate(vSntpTaskCb, "vSntpTaskCb", 2048, NULL, NTP_TASK_PRIORITY, NULL);
}