# Define the executable for the Pico Micro-ROS example
add_executable(07_cartpole
    main.c
)

# Link against Pico, FreeRTOS, microros, and shared utilities
target_link_libraries(07_cartpole PRIVATE
    pico_stdlib
    common_utils
    hardware_uart
    hardware_adc # need
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
    microros
)

# Configure Pico IO settings
pico_enable_stdio_usb(07_cartpole 1)
pico_enable_stdio_uart(07_cartpole 0)

# Compile definitions for Pico config
target_compile_definitions(07_cartpole PRIVATE
    PICO_UART_ENABLE_CRLF_SUPPORT=0
    PICO_STDIO_ENABLE_CRLF_SUPPORT=0
    PICO_STDIO_DEFAULT_CRLF=0
)

# Set UF2 output directory
set_target_properties(07_cartpole PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Generate UF2 and binary outputs
pico_add_extra_outputs(07_cartpole)
