cmake_minimum_required(VERSION 3.8)
project(waveforms)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(fpga_interfaces REQUIRED)
find_package(targeting_interfaces REQUIRED)

# Install get_default_waveform executable.

add_executable(get_default_waveform
  src/nodes/get_default_waveform.cpp
  src/lib/memory_utils.cpp
  include/memory_utils.h
  src/lib/scheduling_utils.cpp
  include/scheduling_utils.h)

target_include_directories(get_default_waveform
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(get_default_waveform rclcpp std_msgs fpga_interfaces targeting_interfaces)

# Install reverse_polarity executable.

add_executable(reverse_polarity
  src/nodes/reverse_polarity.cpp
  src/lib/memory_utils.cpp
  include/memory_utils.h
  src/lib/scheduling_utils.cpp
  include/scheduling_utils.h)

target_include_directories(reverse_polarity
PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

ament_target_dependencies(reverse_polarity rclcpp std_msgs fpga_interfaces targeting_interfaces)


install(TARGETS
  get_default_waveform
  reverse_polarity
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
