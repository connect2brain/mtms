cmake_minimum_required(VERSION 3.12)
project(eeg_processor)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(mtms_interfaces REQUIRED)
find_package(fpga_interfaces REQUIRED)

find_package(OpenMP)

set(MATLAB_FIND_DEBUG true)
# MATLAB

find_package(Matlab)
message(STATUS "MATLAB ROOTDIR: ${Matlab_ROOT_DIR}")
if (Matlab_FOUND)
    # Following 5 lines taken from https://stackoverflow.com/questions/8880802/cmake-linking-shared-library
    # Fixes runtime error "error while loading shared libraries: libMatlabDataArray.so: cannot open shared object file: No such file or directory"
    SET(CMAKE_SKIP_BUILD_RPATH FALSE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib64")
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib64")

    set(LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${Matlab_ROOT_DIR}/extern/bin/glnxa64:${Matlab_ROOT_DIR}/sys/os/glnxa64)
    include_directories(${Matlab_ROOT_DIR}/extern/include/)
    link_directories(${Matlab_ROOT_DIR}/extern/bin/glnxa64)
else ()
    message(STATUS "MATLAB NOT FOUND")
endif ()

add_executable(
        eeg_processor

        src/processor.h
        src/eeg_processor.h
        src/fpga_event.h
        src/eeg_processor.cpp

        src/compiled_matlab_processor/compiled_matlab_processor.h
        src/compiled_matlab_processor/compiled_matlab_processor.cpp
        src/compiled_matlab_processor/matlab_processor_interface.h
        src/compiled_matlab_processor/matlab_processor_interface.cpp

        src/cpp_processor/cpp_processor.h
        src/cpp_processor/cpp_processor.cpp
        src/cpp_processor/cpp_processor_interface.h
        src/cpp_processor/cpp_processor_interface.cpp

        src/python_processor/python_processor.h
        src/python_processor/python_processor.cpp

        src/matlab_processor/matlab_processor.h
        src/matlab_processor/matlab_processor.cpp

        src/utils/scheduling_utils.h
        src/utils/scheduling_utils.cpp
        src/utils/memory_utils.h
        src/utils/memory_utils.cpp
        src/utils/matlab_helpers.h
        src/utils/matlab_helpers.cpp

)

target_include_directories(eeg_processor
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:lib>)

target_include_directories(eeg_processor
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/utils>
        $<INSTALL_INTERFACE:utils>
        )

target_include_directories(eeg_processor
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>
        $<INSTALL_INTERFACE:>
        )

ament_target_dependencies(eeg_processor rclcpp std_msgs mtms_interfaces fpga_interfaces)

if (OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found!")
    target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)
endif ()

if (Matlab_FOUND)

    target_sources(
            eeg_processor
            PRIVATE
            src/matlab_processor/matlab_processor.h
            src/matlab_processor/matlab_processor.cpp
    )

    # MATLAB, Linker to libMatlabEngine in link_directories
    target_link_libraries(eeg_processor MatlabDataArray)
    target_link_libraries(eeg_processor MatlabEngine)

    target_compile_definitions(eeg_processor PUBLIC MATLAB_FOUND)

endif ()

# Python
find_package(PythonLibs)
if (PYTHONLIBS_FOUND)
    message(STATUS "Python found")

    target_sources(
            eeg_processor
            PRIVATE
            src/python_processor/python_processor.h
            src/python_processor/python_processor.cpp
    )

    include_directories(${PYTHON_INCLUDE_DIRS})
    target_link_libraries(eeg_processor ${PYTHON_LIBRARIES})

    target_compile_definitions(eeg_processor PUBLIC PYTHON_FOUND)

else ()
    message(STATUS "Python not found")
endif ()

install(TARGETS
        eeg_processor
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
