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
)

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
)

target_include_directories(safety_monitor_bridge
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(safety_monitor_bridge rclcpp std_msgs fpga_interfaces)

# Install set power handler.

add_executable(set_power_handler
  src/nodes/set_power_handler.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
)

target_include_directories(set_power_handler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(set_power_handler rclcpp std_msgs fpga_interfaces)

# Install stimulation pulse event handler.

add_executable(stimulation_pulse_event_handler
  src/nodes/stimulation_pulse_event_handler.cpp
  src/lib/serdes.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
)

target_include_directories(stimulation_pulse_event_handler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(stimulation_pulse_event_handler rclcpp std_msgs fpga_interfaces)

# Install trigger out pulse event handler.

add_executable(trigger_out_pulse_event_handler
  src/nodes/trigger_out_pulse_event_handler.cpp
  src/lib/serdes.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
)

target_include_directories(trigger_out_pulse_event_handler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(trigger_out_pulse_event_handler rclcpp std_msgs fpga_interfaces)

# Install send event trigger handler.

add_executable(event_trigger_handler
  src/nodes/event_trigger_handler.cpp
  src/lib/fpga.cpp
  src/lib/NiFpga.c
)

target_include_directories(event_trigger_handler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(event_trigger_handler rclcpp std_msgs fpga_interfaces)


install(TARGETS
  run_fpga
  discharge_controller_bridge
  safety_monitor_bridge
  set_power_handler
  stimulation_pulse_event_handler
  trigger_out_pulse_event_handler
  event_trigger_handler
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
