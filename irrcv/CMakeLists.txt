set(COMPONENT_SRCS "irrcv.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_PRIV_INCLUDEDIRS "lib/include")
set(COMPONENT_REQUIRES "ir_rx" "freertos")

register_component()
