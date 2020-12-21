#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)


ifdef CONFIG_COMPONENT_PCF8574
COMPONENT_ADD_INCLUDEDIRS := include
COMPONENT_SRCDIRS := .
else
# Disable component
COMPONENT_ADD_INCLUDEDIRS :=
COMPONENT_ADD_LDFLAGS :=
COMPONENT_SRCDIRS :=
endif