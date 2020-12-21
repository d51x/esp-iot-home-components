set(COMPONENT_SRCS "ota_http.c" "ota.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_PRIV_INCLUDEDIRS "lib/include")
set(COMPONENT_REQUIRES "wifi" "freertos")

register_component()
