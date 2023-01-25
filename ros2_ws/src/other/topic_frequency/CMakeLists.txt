cmake_minimum_required(VERSION 3.8)
project(topic_frequency)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(eeg_interfaces REQUIRED)

add_executable(topic_frequency
        src/topic_frequency.cpp
        src/memory_utils.cpp
        src/memory_utils.h
        src/scheduling_utils.cpp
        src/scheduling_utils.h
        )
ament_target_dependencies(topic_frequency rclcpp std_msgs eeg_interfaces)


install(TARGETS
  topic_frequency
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
