cmake_minimum_required(VERSION 3.19)
include(pico_sdk_import.cmake)
project(picow_wifi C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()


add_executable(picowebserver
        picowserver.c
        )
target_compile_definitions(picowebserver PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )
target_include_directories(picowebserver PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )
target_link_libraries(picowebserver
        pico_cyw43_arch_lwip_poll
        pico_stdlib
        )
pico_enable_stdio_usb(picowebserver 1)        
pico_add_extra_outputs(picowebserver)
