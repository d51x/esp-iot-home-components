
# componet standalone mode

    if(CONFIG_COMPONENT_MCP23017)
        set(COMPONENT_SRCS "mcp23017_http.c" "mcp23017_mqtt.c" "mcp23017.c")

        set(COMPONENT_ADD_INCLUDEDIRS ". include")
    else()
        set(COMPONENT_SRCS "")
        set(COMPONENT_ADD_INCLUDEDIRS "")
        message(STATUS "Building empty mcp23017 component due to configuration")
    endif()


# requirements can't depend on config
set(COMPONENT_REQUIRES i2c_bus)

register_component()
