if(CONFIG_COMPONENT_LCD2004)
set(COMPONENT_SRCS "lcd2004_http.c" "lcd2004.c")


set(COMPONENT_ADD_INCLUDEDIRS ". include")
else()
set(COMPONENT_ADD_INCLUDEDIRS "")
set(COMPONENT_SRCS "")
endif()

set(COMPONENT_REQUIRES)
register_component()

