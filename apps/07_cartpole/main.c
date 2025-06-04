
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <sensor_msgs/msg/joint_state.h>
#include <geometry_msgs/msg/wrench.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>

#include "pico/stdlib.h"

#include "pico_uart_transports.h"
#include "debug_uart.h"

#define DEBUGx 1

#if defined(DEBUGx) && (DEBUGx == 1)
    #define DEBUGPRINT uart_printf
#else
    #define DEBUGPRINT(...) do {} while (0)
#endif

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){uart_printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);vTaskDelete(NULL);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){uart_printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

#define ARRAY_LEN 8
#define JOINT_DOUBLE_LEN 1

const uint LED_PIN = 25;

rcl_publisher_t publisher_1;
rcl_subscription_t subscriber;
QueueHandle_t state_queue;

sensor_msgs__msg__JointState received_msg;


void pid_thread(void * arg){

	// initialize pub msg
	geometry_msgs__msg__Wrench msg;
	msg.force.y = 0.0;

	// initialize pid controller
	float Kp = 20.0, Ki = 10.0, Kd = 10.0;
	float dt = 0.05;

	double position, error, output;
	double prev_error_ = 0, integral_ = 0, derivative_ = 0;

	while(1){
		DEBUGPRINT("Waiting to recieve\n\r");
		// read state values
		if (xQueueReceive(state_queue, &position, portMAX_DELAY)) {

			DEBUGPRINT("Recieved position: %f\n\r", position);

            // update PID Loop
			error = 0.0 - position;
			integral_ = error * dt;
			derivative_ = (error - prev_error_) / dt;
			output = Kp * error + Ki * integral_ + Kd * derivative_;
			prev_error_ = error;

			// pub
			DEBUGPRINT("Publishing force: %f\n\r", output);
			msg.force.y = output;
			RCSOFTCHECK(rcl_publish(&publisher_1, &msg, NULL))
        }
	}
}

// Callback function
void joint_state_callback(const void *msg_in)
{	
    const sensor_msgs__msg__JointState *msg = (const sensor_msgs__msg__JointState *)msg_in;
	double value = msg->position.data[0];  
	xQueueSend(state_queue, &value, 0);  // Non-blocking send
}

void microros_task(void * arg)
{

	// Wait for agent successful ping for 2 minutes.
	const int timeout_ms = 1000; 
	const uint8_t attempts = 120;
	RCCHECK(rmw_uros_ping_agent(timeout_ms, attempts) != RCL_RET_OK);

	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	// create node
	rcl_node_t node;
	RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
	RCCHECK(rclc_node_init_default(&node, "multithread_node", "", &support));

	// -----------------------------------------------------------------
	// IMPORTANT: Initialize the JointState message by allocating fields!
	// -----------------------------------------------------------------

	// frame_id field in header (string)
	char joint_states_msg_buffer[8];
	received_msg.header.frame_id.data = joint_states_msg_buffer;
	received_msg.header.frame_id.size = 0;
	received_msg.header.frame_id.capacity = 8;

	// string[] name (note: strings are sequences!)
	rosidl_runtime_c__String string_buffer[JOINT_DOUBLE_LEN];
	received_msg.name.data = string_buffer;
	received_msg.name.size = 0;
	received_msg.name.capacity = JOINT_DOUBLE_LEN;

	// float64[] position
	double joint_states_position_buffer[JOINT_DOUBLE_LEN];
	//received_msg.position.data = (double*) malloc(JOINT_DOUBLE_LEN * sizeof(double)); <---- Dont do this i FreeRTOS!
	received_msg.position.data = joint_states_position_buffer;
	received_msg.position.size = 0;
	received_msg.position.capacity = JOINT_DOUBLE_LEN;

	// float64[] velocity
	double joint_states_velocity_buffer[JOINT_DOUBLE_LEN];
	received_msg.velocity.data = joint_states_velocity_buffer;
	received_msg.velocity.size = 0;
	received_msg.velocity.capacity = JOINT_DOUBLE_LEN;

	// float64[] effort
	double joint_states_effort_buffer[JOINT_DOUBLE_LEN];
	received_msg.effort.data = joint_states_effort_buffer;
	received_msg.effort.size = 0;
	received_msg.effort.capacity = JOINT_DOUBLE_LEN;

	// initialize string[] name strings (an array of strings!)
	char joint_name_msg_buffer[JOINT_DOUBLE_LEN][ARRAY_LEN];
	for(int i = 0; i < JOINT_DOUBLE_LEN; i++){
		received_msg.name.data[i].data = joint_name_msg_buffer[i];
		received_msg.name.data[i].size = 0;
		received_msg.name.data[i].capacity = ARRAY_LEN;
	}

	// Initialize subscriber for JointState message
	RCCHECK(rclc_subscription_init_default(
		&subscriber,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
		"/cart/pole_state") != RCL_RET_OK);

	// Create executor to run subscriber
	rclc_executor_t executor;
	RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
	RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &received_msg, &joint_state_callback, ON_NEW_DATA));

	// create force publisher
	RCCHECK(rclc_publisher_init_default(
		&publisher_1,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Wrench),
		"/cart/force"));
	
	// Initialize message queue
	state_queue = xQueueCreate(10, sizeof(double));
	
	// Create control loop task
	xTaskCreate(pid_thread,
		"pid_thread",
		512,
		NULL,
		5,
		NULL);

	while(1){
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
        vTaskDelay(pdMS_TO_TICKS(10));  // Yield to other tasks
	}

	// free resources
	RCCHECK(rcl_publisher_fini(&publisher_1, &node));
	RCCHECK(rcl_node_fini(&node));

  	vTaskDelete(NULL);
}

int main(void)
{

	// setup LED PIN
	gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

	initialize_debug_uart();
	sleep_ms(5000);

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

    // Create Micro-ROS FreeRTOS task
	TaskHandle_t task;
    xTaskCreate(microros_task, "MicroROS_Task", 5000, NULL, 1, &task);

	// Bind the task to a single core
	vTaskCoreAffinitySet(task, 1);

	vTaskStartScheduler();

	// The program should never reach here
	while (1){
	}
}

