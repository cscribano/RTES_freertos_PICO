# Build a static library for shared/common utilities
add_library(common_utils STATIC
    pico_uart_transport.c
    debug_uart.c
)

target_include_directories(common_utils PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PICO_SDK_PATH}/src/rp2_common/hardware_uart/include/
)

# Link against Pico SDK for pico_uart_transport.c
target_link_libraries(common_utils PUBLIC 
    pico_stdlib_headers
    FreeRTOS-Kernel
)

# Compile definitions for Pico config (SUPER IMPORTANT)
target_compile_definitions(common_utils PRIVATE
    PICO_UART_ENABLE_CRLF_SUPPORT=0
    PICO_STDIO_ENABLE_CRLF_SUPPORT=0
    PICO_STDIO_DEFAULT_CRLF=0
)