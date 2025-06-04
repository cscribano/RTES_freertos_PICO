# Define the executable for the FreeRTOS example
add_executable(hello_freertos
    main.c
)

# Link against Pico, FreeRTOS
target_link_libraries(hello_freertos
    pico_stdlib
    pico_async_context_freertos
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
)

# Set UF2 output directory
set_target_properties(hello_freertos PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Generate UF2 and binary outputs
pico_add_extra_outputs(hello_freertos)
