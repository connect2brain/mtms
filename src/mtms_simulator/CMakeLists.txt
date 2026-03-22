cmake_minimum_required(VERSION 3.8)
project(mtms_simulator)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(mtms_event_interfaces REQUIRED)
find_package(mtms_waveform_interfaces REQUIRED)
find_package(mtms_device_interfaces REQUIRED)
find_package(mtms_system_interfaces REQUIRED)

# Install mtms_simulator executable.

add_executable(mtms_simulator
  src/mtms_simulator.cpp
  src/channel.cpp
  src/neurone_trigger_client.cpp
)

target_include_directories(mtms_simulator
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(
  mtms_simulator
  rclcpp
  std_msgs
  std_srvs
  mtms_event_interfaces
  mtms_waveform_interfaces
  mtms_device_interfaces
  mtms_system_interfaces
)

install(TARGETS
  mtms_simulator
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
