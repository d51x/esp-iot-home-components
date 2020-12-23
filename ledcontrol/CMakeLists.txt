set(COMPONENT_SRCS "ledcontrol_mqtt.c" "ledcontrol_http.c" "ledcontrol.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_PRIV_INCLUDEDIRS "lib/include")
set(COMPONENT_REQUIRES "gpio" "pwm" "freertos")

register_component()
