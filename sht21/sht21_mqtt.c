#include "sht21_mqtt.h"

#ifdef CONFIG_SENSOR_SHT21
static void sht21_mqtt_send_temp(char **payload, void *args)
{
    sprintf(*payload, "%0.2f", sht21_get_temp());
}

static void sht21_mqtt_send_hum(char **payload, void *args)
{
    sprintf(*payload, "%0.2f", sht21_get_hum());
}



void sht21_mqtt_init()
{
    mqtt_add_periodic_publish_callback( SHT21_MQTT_SEND_TOPIC_TEMP, sht21_mqtt_send_temp, NULL );
    mqtt_add_periodic_publish_callback( SHT21_MQTT_SEND_TOPIC_HUM, sht21_mqtt_send_hum, NULL  );    
}
#endif