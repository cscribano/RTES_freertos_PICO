cmake_minimum_required(VERSION 3.12)

# Import Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Initialize the Pico SDK
pico_sdk_init()

# Project definition
project(pico_micro_ros_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pull in FreeRTOS kernel path and import
set(ENV{FREERTOS_KERNEL_PATH} "${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel")
include(FreeRTOS_Kernel_import.cmake)
add_compile_definitions(PLATFORM_NAME_FREERTOS)

# Include FreeRTOS header directories.
include_directories(
    ${CMAKE_SOURCE_DIR}  # This ensures FreeRTOSConfig.h is found
    ${CMAKE_SOURCE_DIR}/libmicroros/include # microros
)

# Set compile options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -finline-functions -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finline-functions -O2")

# Link to Micro-ROS
link_directories(libmicroros)

# Add student/common modules
add_subdirectory(apps/common)
add_subdirectory(apps/01_executor_pub)
add_subdirectory(apps/02_multithread_pub)
add_subdirectory(apps/04_pubsub)
add_subdirectory(apps/05_isr_pub)
add_subdirectory(apps/06_sensor_pub)
add_subdirectory(apps/07_cartpole)
