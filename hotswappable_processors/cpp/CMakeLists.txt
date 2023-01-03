cmake_minimum_required(VERSION 3.8)
project(processor_factory VERSION 0.0.1 DESCRIPTION "testing")
set(CMAKE_CXX_STANDARD 20)


if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

file(GLOB processor_src
        "src/*.h"
        "src/*.cpp"
        )

add_library(${PROJECT_NAME} SHARED ${processor_src})
add_executable(${PROJECT_NAME}_exec ${processor_src})

if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found!")
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(${PROJECT_NAME}_exec PUBLIC OpenMP::OpenMP_CXX)
endif()

# Configure the directories to search for header files.
target_include_directories(${PROJECT_NAME} PRIVATE source)
# Set the version property.
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
# Set the shared object version property to the project's major version.
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
# Set the public header property to the one with the actual API.
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER processor_factory.h)
