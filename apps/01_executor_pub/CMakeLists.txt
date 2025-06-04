# Define the executable for the Pico Micro-ROS example
add_executable(01_executor_pub
    main.c
)

# Link against Pico, FreeRTOS, microros, and shared utilities
target_link_libraries(01_executor_pub
    pico_stdlib
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
    microros
    common_utils
)

# Configure Pico IO settings
pico_enable_stdio_usb(01_executor_pub 1)
pico_enable_stdio_uart(01_executor_pub 0)


# Compile definitions for Pico config
target_compile_definitions(01_executor_pub PRIVATE
    PICO_UART_ENABLE_CRLF_SUPPORT=0
    PICO_STDIO_ENABLE_CRLF_SUPPORT=0
    PICO_STDIO_DEFAULT_CRLF=0
)

# Set UF2 output directory
set_target_properties(01_executor_pub PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Generate UF2 and binary outputs
pico_add_extra_outputs(01_executor_pub)
