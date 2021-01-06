#
# Component Makefile
#

COMPONENT_SRCDIRS := . lib
COMPONENT_ADD_INCLUDEDIRS := include

EXTRA_COMPONENT_DIRS := components
COMPONENT_EMBED_FILES := 	${PROJECT_PATH}/${EXTRA_COMPONENT_DIRS}/web/favicon.ico \
							${PROJECT_PATH}/${EXTRA_COMPONENT_DIRS}/web/main.min.css \
							${PROJECT_PATH}/${EXTRA_COMPONENT_DIRS}/web/ajax.min.js	\
							${PROJECT_PATH}/${EXTRA_COMPONENT_DIRS}/web/mqtt.min.js	\
							${PROJECT_PATH}/${EXTRA_COMPONENT_DIRS}/web/rgb.min.js	\
							${PROJECT_PATH}/${EXTRA_COMPONENT_DIRS}/web/menu.png	\
							${PROJECT_PATH}/${EXTRA_COMPONENT_DIRS}/web/menu2.png