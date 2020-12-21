if(CONFIG_COMPONENT_PCF8574)
set(COMPONENT_SRCS "pcf8574.c")


set(COMPONENT_ADD_INCLUDEDIRS ". include")
else()
set(COMPONENT_ADD_INCLUDEDIRS "")
set(COMPONENT_SRCS "")
message(STATUS "Building empty pcf8574 component due to configuration")
endif()

set(COMPONENT_REQUIRES i2c_bus)
register_component()
