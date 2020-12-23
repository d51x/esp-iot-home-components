
set(COMPONENT_SRCS "httpd.c")


set(COMPONENT_ADD_INCLUDEDIRS ". include")

set(EXTRA_COMPONENT_DIRS "components")

set(COMPONENT_EMBED_TXTFILES 	${PROJECT_PATH}/${EXTRA_COMPONENT_DIRS}/web/favicon.ico
								${PROJECT_PATH}/${EXTRA_COMPONENT_DIRS}/web/menu.png
								${PROJECT_PATH}/${EXTRA_COMPONENT_DIRS}/web/menu2.png
								${PROJECT_PATH}/${EXTRA_COMPONENT_DIRS}/web/main.min.css
                                ${PROJECT_PATH}/${EXTRA_COMPONENT_DIRS}/web/ajax.min.js )
                                
register_component()
