#include "dht.h"

static const char *TAG = "DHT";

static inline float scale_humidity(dht_type_t _sensor_type, int *data) {
	if(_sensor_type == DHT11) {
		return data[0];
	} else {
		float humidity = data[0] * 256 + data[1];
		return humidity /= 10;
	}
}

static inline float scale_temperature(dht_type_t _sensor_type, int *data) {
	if(_sensor_type == DHT11) {
		return data[2];
	} else {
		float temperature = data[2] & 0x7f;
		temperature *= 256;
		temperature += data[3];
		temperature /= 10;
		if (data[2] & 0x80)
			temperature *= -1;
		return temperature;
	}
}

void ICACHE_FLASH_ATTR dht_init(dht_t *dht)
{
    gpio_config_t gpio_conf;
    gpio_conf.pin_bit_mask = 1ULL << dht->pin;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    
    gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&gpio_conf);

}

esp_err_t ICACHE_FLASH_ATTR dht_read(dht_t *dht){
   
	int counter = 0;
	int laststate = 1;
	int i = 0;
	int j = 0;
	int checksum = 0;
	int data[100];
	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

	// Wake up device, 250ms of high
    gpio_set_direction(dht->pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dht->pin, 1);    
	////sleepms(250);
    ets_delay_us(DHT_WAKEUP_DELAY);

    // Hold low for 20ms
    gpio_set_level(dht->pin, 0);     
    ets_delay_us( DHT_HOLDLOW_DELAY );

    // High for 40ns
    taskENTER_CRITICAL();
    gpio_set_level(dht->pin, 1); 
    ets_delay_us( DHT_HOLDHIGH_DELAY );
    taskEXIT_CRITICAL();  

	// Set DHT_PIN pin as an input
    gpio_set_direction(dht->pin, GPIO_MODE_INPUT);

	// wait for pin to drop?
	while (gpio_get_level(dht->pin) == 1 && i < DHT_MAXCOUNT) {
		os_delay_us(1);
		i++;
	}

	if (i == DHT_MAXCOUNT) {
        ESP_LOGE(TAG, "DHT (gpio%d): Failed to get reading, dying", dht->pin);
		return ESP_FAIL;
    }    
taskENTER_CRITICAL();
    // read data
    for (i = 0; i < DHT_MAXTIMINGS; i++) {
        // Count high time (in approx us)
        counter = 0;
        
        while (gpio_get_level(dht->pin) == laststate) {
            counter++;
            os_delay_us(1);
            if (counter == 1000) break;
        }
        
        laststate = gpio_get_level(dht->pin);
        if (counter == 1000) break;
        
        // store data after 3 reads
        if ((i>3) && (i%2 == 0)) {
                // shove each bit into the storage bytes
                data[j/8] <<= 1;
                if (counter > DHT_BREAKTIME)
                        data[j/8] |= 1;
                j++;
        }
    }    
taskEXIT_CRITICAL();
    if (j >= 39) {
            checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
            ESP_LOGV(TAG, "DHT: %02x %02x %02x %02x [%02x] CS: %02x", data[0], data[1],data[2],data[3],data[4],checksum);
            if (data[4] == checksum) {
                    // checksum is valid
                    dht->temp = scale_temperature(dht->type, data);
                    dht->hum =  scale_humidity(dht->type, data);
                    //ESP_LOGD("DHT (gpio%d): Temperature =  %d *C, Humidity = %d %%\n", dht->pin, (int)(reading.temperature * 100), (int)(reading.humidity * 100));
                    return ESP_OK;
            } else {
                    ESP_LOGE(TAG, "DHT (gpio%d): Checksum was incorrect after %d bits. Expected %d but got %d", dht->pin, j, data[4], checksum);
                    return ESP_FAIL;
            }
    } else {
            ESP_LOGE(TAG, "DHT (gpio%d): Got too few bits: %d should be at least 40", dht->pin, j);
            return ESP_FAIL;
    }
}