set(OUTPUT_NAME linux_basic_request)

add_executable(${OUTPUT_NAME} linux_basic_request.cpp)

target_link_libraries(${OUTPUT_NAME}
        pico_http_client
)