cmake_minimum_required(VERSION 3.8)
project(data_processor)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(mtms_interfaces REQUIRED)
find_package(fpga_interfaces REQUIRED)

find_package(PythonLibs REQUIRED) # find_package(Python COMPONENTS Development REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

add_executable(data_processor src/data_processor.cpp src/processor.cpp src/headers/processor.h src/python_processor.cpp src/headers/python_processor.h src/headers/scheduling_utils.h src/headers/scheduling_utils.cpp)
ament_target_dependencies(data_processor rclcpp std_msgs mtms_interfaces fpga_interfaces)

target_link_libraries(data_processor ${PYTHON_LIBRARIES})

install(TARGETS
  data_processor
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
