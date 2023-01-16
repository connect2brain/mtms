# TODO: ExecutionCondition is naturally an enum. However, enums are not
# yet supported by ROS2, therefore simulate it by using constants
# (see link below for an example). Change to actual enum when
# it is available in ROS2.
#
# https://github.com/ros2/rosidl/issues/260

# Execution condition determines when the event is executed:

# Executed when the time counter of the mTMS device reaches time_us (measured from the start of experiment).
#
uint8 TIMED=0

# Executed when the mTMS device receives an external trigger or an event trigger message.
#
uint8 TRIGGER=1

# Executed as soon as possible.
#
uint8 INSTANT=2

uint8 value
