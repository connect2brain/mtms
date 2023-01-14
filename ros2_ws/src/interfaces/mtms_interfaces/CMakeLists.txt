cmake_minimum_required(VERSION 3.8)
project(mtms_interfaces)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(geometry_msgs REQUIRED)

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
  "msg/EulerAngles.msg"
  "msg/Event.msg"
  "msg/PoseUsingEulerAngles.msg"
  "msg/Target.msg"
  "msg/PlannerState.msg"
  "msg/EegDatapoint.msg"
  "msg/EegBatch.msg"
  "msg/ChannelInfo.msg"
  "msg/Experiment.msg"
  "msg/PulseSequence.msg"
  "msg/Pulse.msg"
  "msg/Train.msg"
  "srv/Target/AddTarget.srv"
  "srv/Target/ChangeComment.srv"
  "srv/Target/RemoveTarget.srv"
  "srv/Target/RenameTarget.srv"
  "srv/Target/ChangeTargetIndex.srv"
  "srv/Target/SetTarget.srv"
  "srv/Target/ToggleSelectTarget.srv"
  "srv/Target/ToggleVisible.srv"
  "srv/Target/SetTargetOrientation.srv"
  "srv/PulseSequence/AddPulseSequence.srv"
  "srv/PulseSequence/RenamePulseSequence.srv"
  "srv/PulseSequence/ToggleSelectPulseSequence.srv"
  "srv/PulseSequence/RemovePulseSequence.srv"
  "srv/PulseSequence/RemovePulse.srv"
  "srv/PulseSequence/SetPulseSequenceIsi.srv"
  "srv/PulseSequence/SetPulseSequenceIntensity.srv"
  "srv/PulseSequence/SetPulseIntensity.srv"
  "srv/PulseSequence/SetPulseIsi.srv"
  "srv/PulseSequence/ToggleSelectPulse.srv"
  "srv/PulseSequence/ToggleVisiblePulse.srv"
  "srv/PulseSequence/ChangePulseIndex.srv"
  "srv/PulseSequence/AddPulseToPulseSequence.srv"
  "msg/Trigger.msg"
  "srv/TriggerPulse.srv"
  "srv/ToggleNavigation.srv"
  "srv/StartPulseSequence.srv"
  "srv/GetChannelVoltages.srv"
  "srv/ClearState.srv"

  "action/AnalyzeMep.action"
  "srv/AnalyzeMepService.srv"
  DEPENDENCIES geometry_msgs
 )

ament_package()
