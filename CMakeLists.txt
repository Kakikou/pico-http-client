cmake_minimum_required(VERSION 3.12)

set(TARGET "" CACHE STRING "LINUX or PICO")

if ("${TARGET}" STREQUAL "PICO")
    set(PICO_BOARD "pico_w")
    set(PICO_SDK_FETCH_FROM_GIT 1)

    include(pico_sdk_import.cmake)
endif ()


project(PicoHttpClient C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if ("${TARGET}" STREQUAL "PICO")
    # Initialize the SDK
    pico_sdk_init()
endif ()

include(pico_http_client_import.cmake)
add_subdirectory(example/basic_request)
