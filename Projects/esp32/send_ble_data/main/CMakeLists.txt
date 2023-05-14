set(SOURCES 
            main.c
            wifi.c
            http_request.c
            ble.c
            flash.c)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ".")
