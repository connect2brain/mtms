cmake_minimum_required(VERSION 3.8)
project(fpga_interfaces)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # uncomment the line when a copyright and license is not present in all source files
  #set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # uncomment the line when this package is not in a git repo
  #set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
"msg/event/ChargeEvent.msg"
"msg/event/DischargeEvent.msg"
"msg/event/StimulationPulsePiece.msg"
"msg/event/StimulationPulseEvent.msg"
"msg/event/TriggerOutPulseEvent.msg"
"msg/event/EventInfo.msg"

"msg/SafetyMonitorErrors.msg"
"msg/SafetyMonitorState.msg"
"msg/DischargeControllerErrors.msg"
"msg/DischargeControllerState.msg"
"msg/DischargeControllerStates.msg"
"msg/Version.msg"

"srv/SetPower.srv"
"srv/SendChargeEvent.srv"
"srv/SendDischargeEvent.srv"
"srv/SendEventTrigger.srv"
"srv/SendStimulationPulseEvent.srv"
"srv/SendTriggerOutPulseEvent.srv"

"srv/StartDevice.srv"
"srv/StopDevice.srv"
"srv/StartExperiment.srv"
"srv/StopExperiment.srv"

)

ament_package()
