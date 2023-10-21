set(OUTPUT_NAME picow_basic_request)
add_executable(${OUTPUT_NAME} picow_basic_request.cpp)

target_link_libraries(${OUTPUT_NAME}
        pico_stdlib
        pico_http_client
)

pico_enable_stdio_uart(${OUTPUT_NAME} 1)
pico_enable_stdio_usb(${OUTPUT_NAME} 0)

pico_add_extra_outputs(${OUTPUT_NAME})