# Define the executable for the FreeRTOS example
add_executable(03_events
    main.c
)

# Link against Pico, FreeRTOS
target_link_libraries(03_events
    pico_stdlib
    pico_async_context_freertos
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
)

# Configure Pico IO settings
pico_enable_stdio_usb(03_events 1)
pico_enable_stdio_uart(03_events 0)

# Set UF2 output directory
set_target_properties(03_events PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Generate UF2 and binary outputs
pico_add_extra_outputs(03_events)
