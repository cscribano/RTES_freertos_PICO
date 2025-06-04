#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/float32.h>

#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "pico_uart_transports.h"
#include "debug_uart.h"

// Define the GPIO pin for the button
#define BUTTON_GPIO 14
const uint LED_PIN = 25;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){uart_printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);vTaskDelete(NULL);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){uart_printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

QueueHandle_t xQueue;
rcl_publisher_t publisher_1;

static void vSenderTask( void *pvParameters ){

	BaseType_t xStatus;
	float temperature;
	uint16_t raw;

	for( ;; ){
		// Read raw ADC value (12-bit, 0-4095)
        raw = adc_read();

        // Convert raw ADC to voltage
        temperature = raw * 3.3f / 4095.0f;
        temperature = 27.0f - (temperature - 0.706f) / 0.001721f; // Convert voltage to temperature in Celsius

		xStatus = xQueueSendToBack( xQueue, &temperature, 0 );
		if( xStatus != pdPASS ){
			uart_printf( "Could not send to the queue.\r\n" );
		}
	}
}

void publisher_thread(void * arg){
	rcl_publisher_t * pub_pointer;
	pub_pointer = (rcl_publisher_t*) arg;
	BaseType_t xStatus;

	float lReceivedValue;
	std_msgs__msg__Float32 msg;
	msg.data = 0;

	while(1){
		xStatus = xQueueReceive( xQueue, &lReceivedValue, pdMS_TO_TICKS(100) );

		if( xStatus == pdPASS ){
			uart_printf( "Received = %d\n", lReceivedValue );
			msg.data = lReceivedValue;
			RCSOFTCHECK(rcl_publish(pub_pointer, &msg, NULL));
		}
		else{
			uart_printf( "Could not receive from the queue.\r\n" );
		}

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void micro_ros_task(void * arg)
{
	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	// create init options
	rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
	RCCHECK(rcl_init_options_init(&init_options, allocator));
	RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator));
	
	// create node
	rcl_node_t node;
	RCCHECK(rclc_node_init_default(&node, "multithread_node", "", &support));

	// create publisher 1
	RCCHECK(rclc_publisher_init_default(
		&publisher_1,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
		"temperature_pub"));

	// spawn new thread with publisher
	xTaskCreate(publisher_thread,
		"thread_1",
		512,
		( void * ) &publisher_1,
		1,
		NULL);
	
	xTaskCreate( vSenderTask, "Sender1", 512, ( void * ) 100, 1, NULL );


	while(1){
        vTaskDelay(pdMS_TO_TICKS(10));  // Yield to other tasks
	}

	// free resources
	RCCHECK(rcl_publisher_fini(&publisher_1, &node));

	RCCHECK(rcl_node_fini(&node));

  	vTaskDelete(NULL);
}

void main(void){
	// setup LED PIN
	gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

	initialize_debug_uart();
	sleep_ms(5000);

	// Initialize ADC hardware
    adc_init();

    // Select ADC input 4 (internal temperature sensor)
    adc_select_input(4);

	// Initial message once serial is ready
	uart_printf("Sensor queue demo starting up!\n");
	gpio_put(LED_PIN, 1);

	

	rmw_uros_set_custom_transport(
		true,
		NULL,
		pico_serial_transport_open,
		pico_serial_transport_close,
		pico_serial_transport_write,
		pico_serial_transport_read
	);

	// Create queue
	xQueue = xQueueCreate( 5, sizeof( float ) );

	// Create main task
	TaskHandle_t task;
    xTaskCreate(micro_ros_task,
            "uros_task",
            5000,
            NULL,
            1,
            &task);

	// Bind the task to a single core
	vTaskCoreAffinitySet(task, 1);

	vTaskStartScheduler();

	// The program should never reach here
	while (1){
	}
}
