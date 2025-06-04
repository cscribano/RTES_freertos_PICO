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


#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);vTaskDelete(NULL);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

rcl_publisher_t publisher_1;
rcl_publisher_t publisher_2;

void publisher_thread(void * arg)
{	
	rcl_publisher_t * pub_pointer;
	pub_pointer = (rcl_publisher_t*) arg;

	std_msgs__msg__Int32 msg;
	msg.data = 0;
	while(1){
		RCSOFTCHECK(rcl_publish(pub_pointer, &msg, NULL));
		msg.data++;
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
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"int32_publisher_t1"));
	
	RCCHECK(rclc_publisher_init_default(
		&publisher_2,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"int32_publisher_t2"));

	// spawn new thread with publisher
	xTaskCreate(publisher_thread,
		"thread_1",
		512,
		( void * ) &publisher_1,
		1,
		NULL);

	xTaskCreate(publisher_thread,
		"thread_2",
		512,
		( void * ) &publisher_2,
		1,
		NULL);

	while(1){
        vTaskDelay(pdMS_TO_TICKS(10));  // Yield to other tasks
	}

	// free resources
	RCCHECK(rcl_publisher_fini(&publisher_1, &node));
	RCCHECK(rcl_publisher_fini(&publisher_2, &node));

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
