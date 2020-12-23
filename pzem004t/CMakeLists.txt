set(COMPONENT_SRCS "pzem004t.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_PRIV_INCLUDEDIRS "lib/include")
set(COMPONENT_REQUIRES "debug" "freertos" "uart")

register_component()
