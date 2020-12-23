set(COMPONENT_SRCS "button.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_PRIV_INCLUDEDIRS "lib/include")
set(COMPONENT_REQUIRES "gpio" "freertos")

register_component()
