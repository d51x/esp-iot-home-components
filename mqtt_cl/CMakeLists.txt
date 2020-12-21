set(COMPONENT_SRCS "mqtt_cl_http.c" "mqtt_cl.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_PRIV_INCLUDEDIRS "lib/include")
set(COMPONENT_REQUIRES "freertos")

register_component()
