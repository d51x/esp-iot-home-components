set(COMPONENT_SRCS "rgbcontrol_mqtt.c" "rgbcontrol_http.c" "rgbcontrol.c" "colors.c" "effects.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_PRIV_INCLUDEDIRS "lib/include")
set(COMPONENT_REQUIRES "gpio" "pwm" "freertos" "ledcontrol")

register_component()
