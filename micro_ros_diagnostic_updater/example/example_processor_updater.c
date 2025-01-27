// Copyright (c) 2020 - for information on the respective copyright owner
// see the NOTICE file and/or the repository
// https://github.com/micro-ROS/micro_ros_diagnostics.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <unistd.h>

#include <rclc/executor.h>

#include "micro_ros_diagnostic_updater/micro_ros_diagnostic_updater.h"

static int my_diagnostic_temp = 0;

rcl_ret_t
my_diagnostic_temperature(diagnostic_value_t * temp_kv)
{
  // Fake a temperature
  ++my_diagnostic_temp;
  if (my_diagnostic_temp > 99) {
    my_diagnostic_temp -= 0;
  }
  rclc_diagnostic_value_set_int(temp_kv, my_diagnostic_temp);

  // Calculate the diagnostic level
  if (my_diagnostic_temp > 85) {
    rclc_diagnostic_value_set_level(temp_kv, 2);
  } else if (my_diagnostic_temp > 75) {
    rclc_diagnostic_value_set_level(temp_kv, 1);
  } else {
    rclc_diagnostic_value_set_level(temp_kv, 0);
  }
  return RCL_RET_OK;
}

rcl_ret_t
my_diagnostic_load(diagnostic_value_t * load_kv)
{
  // Fake a processor load
  rclc_diagnostic_value_set_int(load_kv, my_diagnostic_temp / 2);

  // Calculate the diagnostic level
  if (my_diagnostic_temp > 46) {
    rclc_diagnostic_value_set_level(load_kv, 1);
  } else {
    rclc_diagnostic_value_set_level(load_kv, 0);
  }
  return RCL_RET_OK;
}

int main(int argc, const char * argv[])
{
  rcl_context_t context = rcl_get_zero_initialized_context();
  rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rcl_ret_t rc;

  // create init_options
  rc = rcl_init_options_init(&init_options, allocator);
  if (rc != RCL_RET_OK) {
    printf("Error rcl_init_options_init.\n");
    return -1;
  }

  // create context
  rc = rcl_init(argc, argv, &init_options, &context);
  if (rc != RCL_RET_OK) {
    printf("Error in rcl_init.\n");
    return -1;
  }

  // create rcl_node
  rcl_node_t my_node = rcl_get_zero_initialized_node();
  rcl_node_options_t node_ops = rcl_node_get_default_options();
  rc = rcl_node_init(
    &my_node,
    "node_0",
    "",
    &context,
    &node_ops);
  if (rc != RCL_RET_OK) {
    printf("Error in rcl_node_init\n");
    return -1;
  }

  // updater
  diagnostic_updater_t updater;
  rc = rclc_diagnostic_updater_init(&updater, &my_node, 17, 42);
  if (rc != RCL_RET_OK) {
    printf("Error in creating diagnostic updater\n");
    return -1;
  }
  diagnostic_task_t task;
  rc = rclc_diagnostic_task_init(
    &task, 0,
    &my_diagnostic_temperature);
  if (rc != RCL_RET_OK) {
    printf("Error in creating diagnostic task\n");
    return -1;
  }
  diagnostic_task_t ltask;
  rc = rclc_diagnostic_task_init(
    &ltask, 1,
    &my_diagnostic_load);
  if (rc != RCL_RET_OK) {
    printf("Error in creating diagnostic website checker\n");
    return -1;
  }

  // adding tasks
  rc = rclc_diagnostic_updater_add_task(
    &updater,
    &task);
  if (rc != RCL_RET_OK) {
    printf("Error in adding diagnostic temp task\n");
    return -1;
  }
  rc = rclc_diagnostic_updater_add_task(
    &updater,
    &ltask);
  if (rc != RCL_RET_OK) {
    printf("Error in adding diagnostic website task\n");
    return -1;
  }

  for (unsigned int i = 0; i < 100; ++i) {
    printf("Publishing processor diagnostics\n");
    rc = rclc_diagnostic_updater_update(&updater);
    if (rc != RCL_RET_OK) {
      printf("Error in publishing temp diagnostics\n");
      return -1;
    }
    sleep(1);
  }

  rclc_diagnostic_updater_fini(&updater, &my_node);
  if (rc != RCL_RET_OK) {
    printf("Error while cleaning up!\n");
    return -1;
  }

  return 0;
}
