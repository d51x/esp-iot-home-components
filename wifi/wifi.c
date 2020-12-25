#include "wifi.h"

/* FreeRTOS event group to signal when we are connected*/



static const char *TAG = "WIFI";

#define WIFI_RETRY_COUNT 50  // после истечения попыток переключиться в режим AP_MODE, а если в нем не было подключений в течении Х минут, попробовать еще раз подключиться к SSID
#define WIFI_RECONNECT_DELAY 5000

static uint32_t retry_num = 0;
static uint32_t reconnect_count = 0;


TimerHandle_t xWiFiReconnectTimer;

uint32_t wifi_get_reconnect_count()
{
    return reconnect_count;
}

static void wifi_set_host_name(){
    const char **hostname = calloc(1, TCPIP_HOSTNAME_MAX_SIZE);
    tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_STA, hostname);
    
    if ( strcmp(wifi_cfg->hostname, "") == ESP_OK ) 
    {
        strcpy(wifi_cfg->hostname, *hostname);
    } else {
        if ( tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, (const char *)wifi_cfg->hostname) != ESP_OK )
        {
            ESP_LOGE(TAG, "fail to set new hostname %s", wifi_cfg->hostname);
        }
    }
    wifi_cfg->mode = WIFI_MODE_STA;
    free(hostname);
}

static void wifi_ap_set_ip()
{
    ESP_LOGD(TAG, "function %s started", __func__);
    
    esp_err_t err = ESP_FAIL;
    if ( ESP_WIFI_AP_IP_ADDR_1 != 0 ) {
        err = tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
        ESP_LOGW(TAG, "tcpip_adapter_dhcps_stop err: %s", esp_err_to_name(err));  

        tcpip_adapter_ip_info_t ip_info;
        memset(&ip_info, 0, sizeof(ip_info));
        IP4_ADDR(&ip_info.ip, ESP_WIFI_AP_IP_ADDR_1, ESP_WIFI_AP_IP_ADDR_2, ESP_WIFI_AP_IP_ADDR_3, ESP_WIFI_AP_IP_ADDR_4);
        IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
        IP4_ADDR(&ip_info.gw, ESP_WIFI_AP_IP_ADDR_1, ESP_WIFI_AP_IP_ADDR_2, ESP_WIFI_AP_IP_ADDR_3, ESP_WIFI_AP_IP_ADDR_4);
        
        err = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info);
        ESP_LOGW(TAG, "tcpip_adapter_set_ip_info err: %s", esp_err_to_name(err));  

        err = tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);
        ESP_LOGW(TAG, "tcpip_adapter_dhcps_start err: %s", esp_err_to_name(err));  
    }

        err = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_AP, ESP_WIFI_AP_SSID);
        ESP_LOGW(TAG, "tcpip_adapter_set_hostname err: %s", esp_err_to_name(err));     
}

static void wifi_connect(){
    ESP_LOGD(TAG, "function %s started", __func__);
    wifi_set_host_name();
    esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
    esp_wifi_connect();
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) 
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        ESP_LOGI(TAG, "WIFI_EVENT: STA_START");
        wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        ESP_LOGI(TAG, "WIFI_EVENT: STA_DISCONNECTED");
        
        reconnect_count++;
        ESP_LOGI(TAG, "Reconnects: %d", reconnect_count);
        
        if (xWiFiReconnectTimer != NULL && xTimerIsTimerActive(xWiFiReconnectTimer) == pdFALSE ) 
        {
            ESP_LOGI(TAG, "Reconnect WiFi timer started");
            xTimerStart(xWiFiReconnectTimer, 0);
        }
        xEventGroupClearBits(xWiFiEventGroup, WIFI_CONNECTED_BIT);

        //TODO: start timer for 5 sec to trying connect
        //if (retry_num < ESP_MAXIMUM_RETRY) {
//            wifi_connect();
//            retry_num++;
//            ESP_LOGI(TAG, "retry to connect to the AP %d time(s)", retry_num);
        //} else {
        //    xEventGroupSetBits(xWiFiEventGroup, WIFI_FAIL_BIT);
        //}
        //ESP_LOGI(TAG,"connect to the AP fail");
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START) 
    {
        ESP_LOGI(TAG, "WIFI EVENT: AP_START");
        wifi_ap_set_ip();
        if ( strcmp(wifi_cfg->hostname, "") == ESP_OK ) 
        {
            strcpy(wifi_cfg->hostname, CONFIG_LWIP_LOCAL_HOSTNAME);
        }
    }
    /*
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STOP) 
    {
        //ESP_LOGD(TAG, "WIFI EVENT: AP_STOP");
    } 
    */   
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        ESP_LOGD(TAG, "WIFI EVENT: AP_STACONNECTED");
        
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        
        ESP_LOGD(TAG, "station: "MACSTR" join, AID = %d", MAC2STR(event->mac), event->aid);        
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        ESP_LOGD(TAG, "WIFI_EVENT: AP_STADISCONNECTED");
        
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        
        ESP_LOGD(TAG, "station: "MACSTR" leave, AID = %d",MAC2STR(event->mac), event->aid);        
        
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ESP_LOGI(TAG, "IP_EVENT: STA_GOT_IP");
        if (xWiFiReconnectTimer != NULL && xTimerIsTimerActive(xWiFiReconnectTimer) == pdTRUE ) 
        {
            ESP_LOGI(TAG, "Reconnect WiFi timer stoped");
            xTimerStop(xWiFiReconnectTimer, 0);
        }

        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        //ESP_LOGD(TAG, "got ip: %s", ip4addr_ntoa(&event->ip_info.ip));
        strcpy(wifi_cfg->ip, ip4addr_ntoa(&event->ip_info.ip));
        ESP_LOGD(TAG, "got ip: %s", wifi_cfg->ip);
        retry_num = 0;
        xEventGroupSetBits(xWiFiEventGroup, WIFI_CONNECTED_BIT);
    }
}
void wifi_init()
{

    wifi_cfg = calloc(1, sizeof(wifi_cfg_t));
    wifi_cfg_load(wifi_cfg);
    strcpy(wifi_cfg->ip, "0.0.0.0");

    tcpip_adapter_init();
    xWiFiEventGroup = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_create_default() );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );


    if ( wifi_cfg->first) {
        wifi_cfg->mode = WIFI_MODE_AP;
        if ( strcmp(ESP_WIFI_SSID, "") == ESP_OK ) {
            // start wifi in AP mode
            wifi_init_ap();
        } else {
            // start wifi in STA mode
            wifi_cfg->first = 0;
            nvs_param_u8_save("wifi", "first", wifi_cfg->first);  
            wifi_init_sta(); 
        }
    } else {
        if ( wifi_cfg->mode == WIFI_MODE_STA ) 
        {
            // start wifi in STA mode
            wifi_init_sta(); 
        } else {
            // start wifi in AP mode
            wifi_init_ap();
        }
    }

}

void wifi_deinit()
{
    ESP_LOGI(TAG, "function %s started", __func__);
    free(wifi_cfg);
}


void vReconnectWiFiCallback(TimerHandle_t xTimer){
    //esp_wifi_get_state();
    wifi_connect();
    retry_num++;   
}

void wifi_init_sta(void) {

    ESP_LOGD(TAG, "function %s started", __func__);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS
        },
    };


    if ( strlen(ESP_WIFI_SSID) == 0) {
        strncpy((char *) wifi_config.sta.ssid, wifi_cfg->ssid, strlen(wifi_cfg->ssid));
        strncpy((char *) wifi_config.sta.password, wifi_cfg->password, strlen(wifi_cfg->password));
    }

    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_set_ps (WIFI_PS_NONE);

    esp_wifi_start();    

    xWiFiReconnectTimer = xTimerCreate( "xWiFiReconnectTimer", WIFI_RECONNECT_DELAY / portTICK_RATE_MS, pdTRUE, NULL, &vReconnectWiFiCallback );

    //EventBits_t bits = xEventGroupWaitBits(xWiFiEventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);    

    //if (bits & WIFI_CONNECTED_BIT) {
    //    ESP_LOGI(TAG, "connected to ap SSID: %s password: %s", ESP_WIFI_SSID, ESP_WIFI_PASS);
    //} else if (bits & WIFI_FAIL_BIT) {
    //    ESP_LOGI(TAG, "Failed to connect to SSID: %s, password: %s", ESP_WIFI_SSID, ESP_WIFI_PASS);

        // switch to SoftAP

    //} else {
    //    ESP_LOGE(TAG, "UNEXPECTED EVENT");
    //}

    
}

void wifi_init_ap(void) {

    ESP_LOGD(TAG, "function %s started", __func__);
    
    wifi_config_t ap_config = {
        .ap = {
            .ssid_hidden = 0,
            .channel = 0,
            .max_connection = 1,
            .beacon_interval = 100
        },
    };
    
    strncpy((char *) ap_config.ap.ssid, ESP_WIFI_AP_SSID, strlen(ESP_WIFI_AP_SSID)+1);
    ap_config.ap.ssid_len = strlen(ESP_WIFI_AP_SSID);

    if (strlen(ESP_WIFI_AP_PASS) == 0) {
        memset(ap_config.ap.password, 0, sizeof(ap_config.ap.password));
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    } else {
        strncpy((char *) ap_config.ap.password, ESP_WIFI_AP_PASS, strlen(ESP_WIFI_AP_PASS) + 1);
        ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    }

    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    //ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP) );
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config);
    
    ESP_ERROR_CHECK(esp_wifi_start() );   
}

void wifi_get_mac(char *mac)
{
    if ( wifi_cfg->mode == WIFI_MODE_STA ) {
        esp_read_mac((uint8_t *)mac, ESP_MAC_WIFI_STA);
    }
    else
    {
        esp_read_mac( (uint8_t *)mac, ESP_MAC_WIFI_SOFTAP);
    }
}

int8_t wifi_get_rssi(){
    wifi_ap_record_t info;
    if( !esp_wifi_sta_get_ap_info(&info) ) {
        return info.rssi;
    }
    return 0;
}

bool isWiFiConnected() {
    EventBits_t bits = xEventGroupGetBits( xWiFiEventGroup );
    return bits & WIFI_CONNECTED_BIT;
}

void wifi_cfg_load(wifi_cfg_t *cfg){

    uint8_t val = 0;
    if ( nvs_param_u8_load("wifi", "first", &val) == ESP_OK ) {
        cfg->first = val;
    } else {
        cfg->first = 1;
    }

    nvs_param_str_load("wifi", "hostname", cfg->hostname);

    if ( cfg->first ) {
        // если первый запуск, то скопируем имя хоста из дефайна
        strcpy(cfg->hostname, ESP_WIFI_HOSTNAME);
    } 

    if ( nvs_param_str_load("wifi", "ssid", cfg->ssid) != ESP_OK ) {
        strcpy(cfg->ssid, ESP_WIFI_SSID);
    }

    if ( nvs_param_str_load("wifi", "passw", cfg->password) != ESP_OK ) {
        strcpy(cfg->password, ESP_WIFI_PASS);
    }

    
    if ( nvs_param_u8_load("wifi", "mode", &val) == ESP_OK ) {
        cfg->mode = val;
    } else {
        if ( cfg->first && strcmp(cfg->ssid, "") ) {
            // первый запуск и ssid пустой
            cfg->mode = WIFI_MODE_AP;
        } else {
            // первый запуск и ssid не пустой
            cfg->mode = WIFI_MODE_STA;   
        }
    }
    char *smode = malloc(4);
    switch(cfg->mode) 
    { 
        case WIFI_MODE_STA: strcpy(smode, "STA"); break; 
        case WIFI_MODE_AP:  strcpy(smode, "AP"); break; 
        default: strcpy(smode, "N/A"); break;
    }

    free(smode);


}

void wifi_cfg_save(const wifi_cfg_t *cfg){
    nvs_param_str_save("wifi", "hostname", cfg->hostname);
    nvs_param_str_save("wifi", "ssid", cfg->ssid);
    nvs_param_str_save("wifi", "passw", cfg->password);
    nvs_param_u8_save("wifi", "mode", cfg->mode);   
    nvs_param_u8_save("wifi", "first", cfg->first);   
}
