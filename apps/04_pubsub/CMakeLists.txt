# Define the executable for the Pico Micro-ROS example
add_executable(04_pubsub
    main.c
)

# Link against Pico, FreeRTOS, microros, and shared utilities
target_link_libraries(04_pubsub
    pico_stdlib
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
    microros
    common_utils
)

# Configure Pico IO settings
pico_enable_stdio_usb(04_pubsub 1)
pico_enable_stdio_uart(04_pubsub 0)

# Compile definitions for Pico config
target_compile_definitions(04_pubsub PRIVATE
    PICO_UART_ENABLE_CRLF_SUPPORT=0
    PICO_STDIO_ENABLE_CRLF_SUPPORT=0
    PICO_STDIO_DEFAULT_CRLF=0
)

# Set UF2 output directory
set_target_properties(04_pubsub PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Generate UF2 and binary outputs
pico_add_extra_outputs(04_pubsub)
