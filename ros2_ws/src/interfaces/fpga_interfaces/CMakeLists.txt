cmake_minimum_required(VERSION 3.8)
project(fpga_interfaces)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

# find dependencies
find_package(ament_cmake REQUIRED)

if (BUILD_TESTING)
    find_package(ament_lint_auto REQUIRED)
    # the following line skips the linter which checks for copyrights
    # uncomment the line when a copyright and license is not present in all source files
    #set(ament_cmake_copyright_FOUND TRUE)
    # the following line skips cpplint (only works in a git repo)
    # uncomment the line when this package is not in a git repo
    #set(ament_cmake_cpplint_FOUND TRUE)
    ament_lint_auto_find_test_dependencies()
endif ()

find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
        "msg/event/Charge.msg"
        "msg/event/feedback/ChargeFeedback.msg"
        "msg/event/feedback/ChargeError.msg"

        "msg/event/Discharge.msg"
        "msg/event/feedback/DischargeFeedback.msg"
        "msg/event/feedback/DischargeError.msg"

        "msg/event/Pulse.msg"
        "msg/event/WaveformPiece.msg"
        "msg/event/WaveformPhase.msg"
        "msg/event/feedback/PulseFeedback.msg"
        "msg/event/feedback/PulseError.msg"

        "msg/event/SignalOut.msg"
        "msg/event/feedback/SignalOutFeedback.msg"
        "msg/event/feedback/SignalOutError.msg"

        "msg/event/Event.msg"
        "msg/event/ExecutionCondition.msg"

        "msg/state/ChannelState.msg"
        "msg/state/ChannelError.msg"
        "msg/state/DeviceState.msg"
        "msg/state/ExperimentState.msg"
        "msg/state/StartupError.msg"
        "msg/state/SystemState.msg"
        "msg/state/SystemError.msg"

        "msg/Settings.msg"
        "msg/Version.msg"

        "srv/SendCharge.srv"
        "srv/SendDischarge.srv"
        "srv/SendEventTrigger.srv"
        "srv/SendPulse.srv"
        "srv/SendSignalOut.srv"

        "srv/StartDevice.srv"
        "srv/StopDevice.srv"
        "srv/StartExperiment.srv"
        "srv/StopExperiment.srv"

        "srv/DisableChecks.srv"
        "srv/SendSettings.srv"
        )

ament_package()
