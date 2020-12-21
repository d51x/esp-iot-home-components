#include "pzem004t_mqtt.h"
#include "iot_debug.h"

#ifdef CONFIG_SENSOR_PZEM004_T

#define VOLTAGE_MQTT_TOPIC_PARAM	"pmv"
#define CURRENT_MQTT_TOPIC_PARAM	"pmc"
#define POWER_MQTT_TOPIC_PARAM		"pmw"
#define ENERGY_MQTT_TOPIC_PARAM		"pmwh"

#ifdef CONFIG_SENSOR_PZEM004_T_CALC_CONSUMPTION
#define ENERGY_CONSUMPTION_MQTT_TOPIC_PARAM		"pme"
#endif

static const char *TAG = "PZEM";

static void pzem_mqtt_send_voltage(char **payload, void *args)
{
    
    #ifdef CONFIG_COMPONENT_DEBUG
	log_rtc_debug_str("pzem_mqtt_send_voltage");
	#endif 

    pzem_data_t pzem_data = pzem_get_data();
    sprintf(*payload, "%0.1f", pzem_data.voltage);
}

static void pzem_mqtt_send_current(char **payload, void *args)
{
    #ifdef CONFIG_COMPONENT_DEBUG
	log_rtc_debug_str("pzem_mqtt_send_current");
	#endif 

    pzem_data_t pzem_data = pzem_get_data();
    sprintf(*payload, "%0.1f", pzem_data.current);
}

static void pzem_mqtt_send_power(char **payload, void *args)
{
    #ifdef CONFIG_COMPONENT_DEBUG
	log_rtc_debug_str("pzem_mqtt_send_power");
	#endif 

    pzem_data_t pzem_data = pzem_get_data();
    sprintf(*payload, "%d", (uint32_t)pzem_data.power);
}

static void pzem_mqtt_send_energy(char **payload, void *args)
{
    #ifdef CONFIG_COMPONENT_DEBUG
	log_rtc_debug_str("pzem_mqtt_send_energy");
	#endif 
    
    pzem_data_t pzem_data = pzem_get_data();
    sprintf(*payload, "%d", (uint32_t)pzem_data.energy);
}

#ifdef CONFIG_SENSOR_PZEM004_T_CALC_CONSUMPTION
static void pzem_mqtt_send_consunption(char **payload, void *args)
{
    #ifdef CONFIG_COMPONENT_DEBUG
    log_rtc_debug_str("pzem_mqtt_send_consunption");
    #endif
    
    pzem_data_t pzem_data = pzem_get_data();
    *payload = (char *) realloc(*payload, 140);

    memset(*payload, 0, 140);
    //WDT_FEED();
    snprintf(*payload, 140, "{"
                                "\"t\":{"
                                    "\"a\":%0.2f,"
                                    "\"n\":%0.2f,"
                                    "\"d\":%0.2f"
                                    "},"
                                "\"y\":{"
                                    "\"a\":%0.2f,"
                                    "\"n\":%0.2f,"
                                    "\"d\":%0.2f"
                                    "}"
                            "}"
                    , pzem_data.consumption.today_total / PZEM_FLOAT_DIVIDER
                    , pzem_data.consumption.today_night / PZEM_FLOAT_DIVIDER
                    , pzem_data.consumption.today_day / PZEM_FLOAT_DIVIDER
                    , pzem_data.consumption.prev_total / PZEM_FLOAT_DIVIDER
                    , pzem_data.consumption.prev_night / PZEM_FLOAT_DIVIDER
                    , pzem_data.consumption.prev_day / PZEM_FLOAT_DIVIDER
    );
}
#endif

void pzem_mqtt_init()
{
    mqtt_add_periodic_publish_callback( VOLTAGE_MQTT_TOPIC_PARAM, pzem_mqtt_send_voltage, NULL );
    mqtt_add_periodic_publish_callback( CURRENT_MQTT_TOPIC_PARAM, pzem_mqtt_send_current, NULL  );    
    mqtt_add_periodic_publish_callback( POWER_MQTT_TOPIC_PARAM, pzem_mqtt_send_power, NULL  );    
    mqtt_add_periodic_publish_callback( ENERGY_MQTT_TOPIC_PARAM, pzem_mqtt_send_energy, NULL  );    

#ifdef CONFIG_SENSOR_PZEM004_T_CALC_CONSUMPTION    
    mqtt_add_periodic_publish_callback( ENERGY_CONSUMPTION_MQTT_TOPIC_PARAM, pzem_mqtt_send_consunption, NULL  );    
#endif    
}
#endif