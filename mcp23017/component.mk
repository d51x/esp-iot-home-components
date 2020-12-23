#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

# componet standalone mode

ifdef CONFIG_COMPONENT_MCP23017
COMPONENT_ADD_INCLUDEDIRS := include
COMPONENT_SRCDIRS := .
else
# Disable component
COMPONENT_ADD_INCLUDEDIRS :=
COMPONENT_ADD_LDFLAGS :=
COMPONENT_SRCDIRS :=
endif

