add_library(pico_http_client INTERFACE)

if ("${PICO_BOARD}" STREQUAL "pico_w")

    target_sources(pico_http_client INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/src/pico_http_client.c
            ${CMAKE_CURRENT_LIST_DIR}/src/socket.c
            ${CMAKE_CURRENT_LIST_DIR}/src/lwip/socket_impl.c)

    target_link_libraries(pico_http_client INTERFACE
            pico_cyw43_arch_lwip_threadsafe_background)

elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")

    target_sources(pico_http_client INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/src/pico_http_client.c
            ${CMAKE_CURRENT_LIST_DIR}/src/socket.c
            ${CMAKE_CURRENT_LIST_DIR}/src/unix/socket_impl.c)

endif ()

target_include_directories(pico_http_client INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
