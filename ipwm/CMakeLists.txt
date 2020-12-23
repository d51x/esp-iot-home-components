set(COMPONENT_SRCS "ipwm.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_PRIV_INCLUDEDIRS "lib/include")
set(COMPONENT_REQUIRES "gpio" "pwm")

register_component()
