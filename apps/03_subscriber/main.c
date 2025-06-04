#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/int32.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>

#include "pico/stdlib.h"
#include "pico_uart_transports.h"

const uint LED_PIN = 25;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);vTaskDelete(NULL);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

rcl_subscription_t subscriber_1;
std_msgs__msg__Int32 msg_sub;


void subscription_callback(const void * msgin)
{
	const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;

	// ros2 topic pub  /int32_subscriber std_msgs/msg/Int32 "{'data': 9}" --once
	if (msg->data > 10){
		gpio_put(LED_PIN, 1);
	}
	else{
		gpio_put(LED_PIN, 0);
	}
	    
}


void micro_ros_task(void * arg)
{
	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	// create init options
	RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

	// create node
	rcl_node_t node;
	RCCHECK(rclc_node_init_default(&node, "multithread_node", "", &support));

	// create subscriber
	RCCHECK(rclc_subscription_init_default(
		&subscriber_1,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"int32_subscriber"));


	// create executor
	rclc_executor_t executor;
	RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
	RCCHECK(rclc_executor_add_subscription(&executor, &subscriber_1, &msg_sub, &subscription_callback, ON_NEW_DATA));

	while(1){
		rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
        vTaskDelay(pdMS_TO_TICKS(10));  // Yield to other tasks
	}

	// free resources
	RCCHECK(rcl_subscription_fini(&subscriber_1, &node));
	RCCHECK(rcl_node_fini(&node));

  	vTaskDelete(NULL);
}

void main(void)
{
	rmw_uros_set_custom_transport(
		true,
		NULL,
		pico_serial_transport_open,
		pico_serial_transport_close,
		pico_serial_transport_write,
		pico_serial_transport_read
	);

	// setup LED PIN
	gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

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
