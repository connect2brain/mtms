cmake_minimum_required(VERSION 3.8)
project(remote_controller)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(mtms_system_interfaces REQUIRED)
find_package(shared_stimulation_interfaces REQUIRED)
find_package(mtms_trial_interfaces REQUIRED)
find_package(mtms_targeting_interfaces REQUIRED)
find_package(mtms_eeg_interfaces REQUIRED)
find_package(realtime_utils REQUIRED)

# Install remote_controller executable.

add_executable(remote_controller
  src/remote_controller.cpp
)

target_include_directories(remote_controller
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(remote_controller
  realtime_utils::utils
)

ament_target_dependencies(
  remote_controller
  rclcpp
  std_msgs
  std_srvs
  mtms_system_interfaces
  shared_stimulation_interfaces
  mtms_trial_interfaces
  mtms_targeting_interfaces
  mtms_eeg_interfaces
  realtime_utils
)

install(TARGETS
  remote_controller
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
