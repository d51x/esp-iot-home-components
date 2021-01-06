#include "ota.h"
#include "nvsparam.h"

static const char *TAG = "OTA";

#define OTA_NVS_SECTION "ota"
#define OTA_NVS_KEY "fw"

static void ota_save_nvs(ota_firm_t *fw)
{
    esp_err_t err = nvs_param_save(OTA_NVS_SECTION, OTA_NVS_KEY, fw, sizeof(ota_firm_t));
    if ( err != ESP_OK ) {
        ESP_LOGE(TAG, "%s: %s", __func__, esp_err_to_name(err));
    }
}

esp_err_t ota_load_nvs(ota_firm_t *fw)
{
    esp_err_t err = nvs_param_load(OTA_NVS_SECTION, OTA_NVS_KEY, fw);
    if ( err != ESP_OK ) {
        ESP_LOGE(TAG, "%s: %s", __func__, esp_err_to_name(err));
    }
    return err;
}

esp_err_t ota_task_upgrade_from_web(httpd_req_t *req, char *err_text){
    ESP_LOGW(TAG, "Starting OTA...");
    
    ota_status.state = OTA_START;
    ota_status.progress = 0;
    
    int total_len = req->content_len;
    int file_len = total_len;

    ota_status.total = file_len;

    char fname[OTA_FILENAME_LENGTH];
    int recv_len;           // принято за раз
    int remain = total_len;  // осталось загрузить
    int received = 0;  // загружено
   
    int is_firts = 0;

    int one_part = total_len / 100;
    int new_part = one_part;
    int buf_size = CONFIG_OTA_BUF_SIZE;  // default


    esp_ota_handle_t ota_handle; 
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

    char *upgrade_data_buf = (char *)malloc( buf_size );
    if (!upgrade_data_buf) {
        strcpy(err_text, "Couldn't allocate memory to upgrade data buffer");
        ESP_LOGE(TAG, err_text);
        ota_status.state = OTA_ERROR;
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGW(TAG, "Start uploading firmware, size %d", total_len);        
    ESP_LOGW(TAG, "upload buffer size is %d", buf_size);        


    esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error With OTA Begin, Cancelling OTA");
        ESP_LOGE(TAG, esp_err_to_name(err));
        strcpy(err_text, esp_err_to_name(err));
        strcpy(err_text+strlen(err_text), "Error With OTA Begin, Cancelling OTA");
        ESP_LOGE(TAG, err_text);
        ota_status.state = OTA_ERROR;
        return ESP_ERR_FLASH_OP_FAIL;               
    } 

    uint8_t retry_count = 0;
    //=========================================================================================
    while (remain > 0) {
        recv_len = httpd_req_recv(req, upgrade_data_buf, remain > buf_size ? buf_size : remain);
        if ( recv_len < 0) {
            if (recv_len == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                if ( retry_count > 20 ) {
                    ota_status.state = OTA_ERROR;
                ESP_LOGE(TAG, "Timeout ... Max retry count reached %d...  httpd_req_recv %d (%d%%)", retry_count, received, received*100/total_len);  
                break;  
                }                
                retry_count++;
                ESP_LOGE(TAG, "Socket timeout, uploaded %d (%d%%)", received, received*100/total_len);        
                vTaskDelay(5 / portTICK_PERIOD_MS);
                continue;
            }
            sprintf(err_text, "File upload failed, uploaded %d%%", received*100/total_len);
            ota_status.state = OTA_ERROR;
            ESP_LOGE(TAG, err_text);      
            return ESP_ERR_FLASH_OP_FAIL;             
        }

        // Is this the first data we are receiving
        // If so, it will have the information in the header we need.
        if ( !is_firts ) {
            is_firts = 1;
            ESP_LOGD(TAG, "Writing first block to OTA partition");
            
            // Lets find out where the actual data staers after the header info	
            
            char *body_start_p = strstr(upgrade_data_buf, "\r\n\r\n") + 4;
            //ESP_LOGW(TAG, "body_start_p = 0x%02X", body_start_p - upgrade_data_buf + 1);

            char *post_data = malloc(body_start_p - upgrade_data_buf + 1);
            memset(post_data, 0 , body_start_p - upgrade_data_buf + 1);
            memcpy(post_data, upgrade_data_buf, body_start_p - upgrade_data_buf);
            //ESP_LOGW(TAG, "%s", post_data);
            post_data = strstr(post_data, "filename=") + 10;
            post_data = copy_str_from_str(post_data, "\"");
            strncpy(fname, post_data, OTA_FILENAME_LENGTH);
            //file_len = body_start_p - upgrade_data_buf;
            file_len = total_len - 196;
            ota_status.total = file_len;
            
            ESP_LOGW(TAG, "%s (%d)", fname, file_len);
            free(post_data);

            int body_part_len = recv_len - (body_start_p - upgrade_data_buf); 
            //ESP_LOGW(TAG, "body_part_len = %d", body_part_len);

            err = esp_ota_write(ota_handle, (const void *)body_start_p, body_part_len);
            if ( err != ESP_OK) {
                strcpy(err_text, esp_err_to_name(err));
                strcpy(err_text+strlen(err_text), "ERROR 1: OTA write failed");
                ESP_LOGE(TAG, err_text);
                ota_status.state = OTA_ERROR;
                return err;
            }    
        } else {
            // Write OTA data
            ota_status.state = OTA_PROGRESS;
            err = esp_ota_write(ota_handle, (const void *)upgrade_data_buf, recv_len);
            if ( err != ESP_OK ) {
                strcpy(err_text, esp_err_to_name(err));
                strcpy(err_text+strlen(err_text), "ERROR 2: OTA write failed");
                ESP_LOGE(TAG, err_text);
                ota_status.state = OTA_ERROR;
                return err;
            }    
        }
        received += recv_len; 
        remain -= recv_len;

        if ( new_part < received ) {
            ESP_LOGI(TAG, "uploaded %d %%", new_part*100/total_len);
            new_part += one_part;
            ota_status.progress = new_part;
        }
    } // while
    //=========================================================================================
    ota_status.progress = file_len;

    free(upgrade_data_buf);
    if (esp_ota_end(ota_handle) == ESP_OK) {
        // Lets update the partition
        
        if (esp_ota_set_boot_partition(update_partition) == ESP_OK) {

            const esp_partition_t *boot_partition = esp_ota_get_boot_partition();
            ESP_LOGI("OTA", "Next boot partition subtype %d at offset 0x%x", boot_partition->subtype, boot_partition->address);
            ESP_LOGI("OTA", "Please Restart System...");

            // save to nvs firmware data: updated datetime, filesize, filename
            ota_firm_t *fw = malloc( sizeof(ota_firm_t));
            strcpy(fw->fname, fname);
            fw->size = file_len; //total_len;
            get_localtime(fw->dt);
            ota_save_nvs(fw);
            free(fw);
            ota_status.state = OTA_FINISH;
        } else {
            strcpy(err_text, "ERROR1: OTA upgrading failed. Flashed Error!");
            ESP_LOGE(TAG, err_text);
            ota_status.state = OTA_ERROR;
            return ESP_ERR_FLASH_OP_FAIL;
        }
    } else {
        strcpy(err_text, esp_err_to_name(err));
        strcpy(err_text+strlen(err_text), "ERROR2: OTA upgrading failed. Flashed Error!");
        ESP_LOGE(TAG, err_text);
        ota_status.state = OTA_ERROR;
        return ESP_ERR_FLASH_OP_FAIL;
    } 
    return ESP_OK;        
}

