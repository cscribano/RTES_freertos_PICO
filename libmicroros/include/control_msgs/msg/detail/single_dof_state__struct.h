// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from control_msgs:msg/SingleDOFState.idl
// generated code does not contain a copyright notice

#ifndef CONTROL_MSGS__MSG__DETAIL__SINGLE_DOF_STATE__STRUCT_H_
#define CONTROL_MSGS__MSG__DETAIL__SINGLE_DOF_STATE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'name'
#include "rosidl_runtime_c/string.h"

// Struct defined in msg/SingleDOFState in the package control_msgs.
typedef struct control_msgs__msg__SingleDOFState
{
  rosidl_runtime_c__String name;
  double reference;
  double feedback;
  double feedback_dot;
  double error;
  double error_dot;
  double time_step;
  double output;
} control_msgs__msg__SingleDOFState;

// Struct for a sequence of control_msgs__msg__SingleDOFState.
typedef struct control_msgs__msg__SingleDOFState__Sequence
{
  control_msgs__msg__SingleDOFState * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} control_msgs__msg__SingleDOFState__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // CONTROL_MSGS__MSG__DETAIL__SINGLE_DOF_STATE__STRUCT_H_
