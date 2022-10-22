cmake_minimum_required(VERSION 3.8)
project(fpga_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(fpga_interfaces REQUIRED)

# Install run_fpga executable.

add_executable(run_fpga
  src/nodes/run_fpga.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
)

target_include_directories(run_fpga
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(run_fpga rclcpp std_msgs fpga_interfaces)

# Install discharge controller bridge.

add_executable(discharge_controller_bridge
  src/nodes/discharge_controller_bridge.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
  src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)

target_include_directories(discharge_controller_bridge
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(discharge_controller_bridge rclcpp std_msgs fpga_interfaces)

# Install safety monitor bridge.

add_executable(safety_monitor_bridge
  src/nodes/safety_monitor_bridge.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
  src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)

target_include_directories(safety_monitor_bridge
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(safety_monitor_bridge rclcpp std_msgs fpga_interfaces)

# Install system state bridge.

add_executable(system_state_bridge
        src/nodes/system_state_bridge.cpp
        src/lib/fpga.cpp
        src/lib/NiFpga.c
        src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)

target_include_directories(system_state_bridge
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

ament_target_dependencies(system_state_bridge rclcpp std_msgs fpga_interfaces)

# Install start device handler.

add_executable(start_device_handler
  src/nodes/start_device_handler.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
  src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)


target_include_directories(start_device_handler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(start_device_handler rclcpp std_msgs fpga_interfaces)

# Install start experiment handler.

add_executable(start_experiment_handler
        src/nodes/start_experiment_handler.cpp
        src/lib/fpga.cpp
        src/lib/NiFpga.c
        src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)


target_include_directories(start_experiment_handler
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

ament_target_dependencies(start_experiment_handler rclcpp std_msgs fpga_interfaces)

# Install stop experiment handler.

add_executable(stop_experiment_handler
        src/nodes/stop_experiment_handler.cpp
        src/lib/fpga.cpp
        src/lib/NiFpga.c
        src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)


target_include_directories(stop_experiment_handler
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

ament_target_dependencies(stop_experiment_handler rclcpp std_msgs fpga_interfaces)

# Install stop device handler.

add_executable(stop_device_handler
        src/nodes/stop_device_handler.cpp
        src/lib/fpga.cpp
        src/lib/NiFpga.c
        src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)


target_include_directories(stop_device_handler
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

ament_target_dependencies(stop_device_handler rclcpp std_msgs fpga_interfaces)

# Install disable checks handler.

add_executable(disable_checks_handler
        src/nodes/disable_checks_handler.cpp
        src/lib/fpga.cpp
        src/lib/NiFpga.c
        src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)


target_include_directories(disable_checks_handler
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

ament_target_dependencies(disable_checks_handler rclcpp std_msgs fpga_interfaces)

# Install stimulation pulse event handler.

add_executable(stimulation_pulse_event_handler
  src/nodes/stimulation_pulse_event_handler.cpp
  src/lib/serdes.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
  src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)

target_include_directories(stimulation_pulse_event_handler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(stimulation_pulse_event_handler rclcpp std_msgs fpga_interfaces)

# Install trigger out pulse event handler.

add_executable(trigger_out_event_handler
  src/nodes/trigger_out_event_handler.cpp
  src/lib/serdes.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
  src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)

target_include_directories(trigger_out_event_handler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(trigger_out_event_handler rclcpp std_msgs fpga_interfaces)

# Install charge event handler.

add_executable(charge_event_handler
  src/nodes/charge_event_handler.cpp
  src/lib/serdes.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
  src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)

target_include_directories(charge_event_handler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(charge_event_handler rclcpp std_msgs fpga_interfaces)

# Install discharge event handler.

add_executable(discharge_event_handler
  src/nodes/discharge_event_handler.cpp
  src/lib/serdes.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
  src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)

target_include_directories(discharge_event_handler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(discharge_event_handler rclcpp std_msgs fpga_interfaces)

# Install send event trigger handler.

add_executable(event_trigger_handler
  src/nodes/event_trigger_handler.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
  src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)

target_include_directories(event_trigger_handler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(event_trigger_handler rclcpp std_msgs fpga_interfaces)

add_executable(feedback_monitor_bridge
  src/nodes/feedback_monitor_bridge.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
  src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)


target_include_directories(feedback_monitor_bridge
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

ament_target_dependencies(feedback_monitor_bridge rclcpp std_msgs fpga_interfaces)

add_executable(settings_handler
        src/nodes/settings_handler.cpp
        src/lib/fpga.cpp
        src/lib/NiFpga.c
        src/lib/memory_utils.cpp include/memory_utils.h src/lib/scheduling_utils.cpp include/scheduling_utils.h)


target_include_directories(settings_handler
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

ament_target_dependencies(settings_handler rclcpp std_msgs fpga_interfaces)


install(TARGETS
  run_fpga
  #discharge_controller_bridge
  #safety_monitor_bridge
  system_state_bridge
  start_device_handler
  stop_device_handler
  start_experiment_handler
  stop_experiment_handler
  charge_event_handler
  discharge_event_handler
  stimulation_pulse_event_handler
  trigger_out_event_handler
  event_trigger_handler
  feedback_monitor_bridge
  disable_checks_handler
  settings_handler
  DESTINATION lib/${PROJECT_NAME})

# Install the launch directory.
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
