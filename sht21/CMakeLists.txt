if(CONFIG_SENSOR_SHT21)
set(COMPONENT_SRCS "sht21_http.c" "sht21.c" "sht21_mqtt.c" "sht21.c sht21_http.c")


set(COMPONENT_ADD_INCLUDEDIRS ". include")
else()
set(COMPONENT_ADD_INCLUDEDIRS "")
set(COMPONENT_SRCS "")
endif()

set(COMPONENT_REQUIRES)
register_component()
