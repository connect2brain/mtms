cmake_minimum_required(VERSION 3.8)
project(eeg_trigger_processor_cpp)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(mtms_interfaces REQUIRED)
find_package(fpga_interfaces REQUIRED)

add_executable(
        eeg_trigger_processor
        src/eeg_trigger_processor.cpp
        src/headers/eeg_trigger_processor.h

        src/memory_utils.cpp
        src/headers/memory_utils.h

        src/scheduling_utils.cpp
        src/headers/scheduling_utils.h
)

target_include_directories(eeg_trigger_processor
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:lib>)

ament_target_dependencies(eeg_trigger_processor rclcpp std_msgs mtms_interfaces fpga_interfaces)


install(TARGETS
        eeg_trigger_processor
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
message("TARGET: ${CMAKE_BUILD_TYPE}")
