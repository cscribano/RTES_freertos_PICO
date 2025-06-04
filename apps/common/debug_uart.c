#include <stdarg.h>
#include <stdio.h>

#include "hardware/uart.h"
#include "hardware/gpio.h"

// Define UART parameters
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define UART_PRINT_BUFFER_SIZE 128

void uart_printf(const char *format, ...) {
    char buffer[UART_PRINT_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, UART_PRINT_BUFFER_SIZE, format, args);
    va_end(args);
    uart_puts(UART_ID, buffer);
}

void initialize_debug_uart(){
    // Initialize chosen UART
    uart_init(UART_ID, BAUD_RATE);

    // Set the GPIO functions for UART pins
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Set UART format if needed (default is 8N1)
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

    // Optional: enable FIFO buffers
    uart_set_fifo_enabled(UART_ID, true);
}
