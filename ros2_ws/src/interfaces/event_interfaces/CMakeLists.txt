cmake_minimum_required(VERSION 3.8)
project(event_interfaces)

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
        "msg/Charge.msg"
        "msg/feedback/ChargeFeedback.msg"
        "msg/feedback/ChargeError.msg"

        "msg/Discharge.msg"
        "msg/feedback/DischargeFeedback.msg"
        "msg/feedback/DischargeError.msg"

        "msg/Pulse.msg"
        "msg/WaveformPiece.msg"
        "msg/WaveformPhase.msg"
        "msg/feedback/PulseFeedback.msg"
        "msg/feedback/PulseError.msg"

        "msg/SignalOut.msg"
        "msg/feedback/SignalOutFeedback.msg"
        "msg/feedback/SignalOutError.msg"

        "msg/Stimulus.msg"

        "msg/EventInfo.msg"
        "msg/ExecutionCondition.msg"

        "srv/SendCharge.srv"
        "srv/SendDischarge.srv"
        "srv/SendEventTrigger.srv"
        "srv/SendPulse.srv"
        "srv/SendSignalOut.srv"
        )

ament_package()
