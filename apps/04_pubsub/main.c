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

rcl_subscription_t subscriber_1;
rcl_publisher_t publisher_1;
rcl_publisher_t publisher_2;

std_msgs__msg__Int32 msg_sub;
std_msgs__msg__Int32 msg_pub;

void thread_1(void * arg)
{
	std_msgs__msg__Int32 msg;
	msg.data = 0;
	while(1){
		RCSOFTCHECK(rcl_publish(&publisher_1, &msg, NULL));
		msg.data++;
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void subscription_callback(const void * msgin)
{
	const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
	printf("Received: %d\n", msg->data);
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
	RCLC_UNUSED(last_call_time);
	if (timer != NULL) {
		RCSOFTCHECK(rcl_publish(&publisher_2, &msg_pub, NULL));
		msg_pub.data++;
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

	// create publisher 1 (separate thread)
	RCCHECK(rclc_publisher_init_default(
		&publisher_1,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"int32_publisher_t1"));
	
	// create publisher 2 (main thread, timer)
	RCCHECK(rclc_publisher_init_default(
		&publisher_2,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"int32_publisher_main"));

	// create timer,
	rcl_timer_t timer;
	const unsigned int timer_timeout = 1000;
	RCCHECK(rclc_timer_init_default(
		&timer,
		&support,
		RCL_MS_TO_NS(timer_timeout),
		timer_callback));

	// create executor
	rclc_executor_t executor;
	RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));
	RCCHECK(rclc_executor_add_subscription(&executor, &subscriber_1, &msg_sub, &subscription_callback, ON_NEW_DATA));
	RCCHECK(rclc_executor_add_timer(&executor, &timer));

	// spawn new thread with publisher
	xTaskCreate(thread_1,
		"thread_1",
		512,
		NULL,
		1,
		NULL);

	// intialize published message
	msg_pub.data = 0;

	while(1){
		rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
        vTaskDelay(pdMS_TO_TICKS(10));  // Yield to other tasks
	}

	// free resources
	RCCHECK(rcl_publisher_fini(&publisher_1, &node));
	RCCHECK(rcl_publisher_fini(&publisher_2, &node));
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
