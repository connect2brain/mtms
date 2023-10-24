cmake_minimum_required(VERSION 3.8)
project(preprocessor)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

find_package(PythonLibs 3 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

find_package(ament_cmake REQUIRED)
find_package(pybind11_vendor REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(std_msgs REQUIRED)
find_package(eeg_interfaces REQUIRED)
find_package(project_interfaces REQUIRED)

add_executable(
        preprocessor

        src/preprocessor.cpp
        src/preprocessor_wrapper.cpp

        src/memory_utils.cpp
        src/scheduling_utils.cpp
)

target_include_directories(preprocessor
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:lib>)

target_link_libraries(preprocessor ${pybind11_LIBRARIES})
target_include_directories(preprocessor PUBLIC ${pybind11_INCLUDE_DIRS})

ament_target_dependencies(
  preprocessor
  rclcpp
  rclcpp_action
  std_msgs
  eeg_interfaces
  project_interfaces
)

install(TARGETS
        preprocessor
        DESTINATION lib/${PROJECT_NAME}
        )

install(
        DIRECTORY launch
        DESTINATION share/${PROJECT_NAME}
)

if (BUILD_TESTING)
    find_package(ament_lint_auto REQUIRED)
    ament_lint_auto_find_test_dependencies()
endif ()

ament_package()
message(STATUS "TARGET: ${CMAKE_BUILD_TYPE}")
