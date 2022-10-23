# Contains information about the event.

uint16 id

# When the event will be executed:
#
# 0: Timed event, event will be executed when FPGA reaches time_us (calculated from the start of experiment)
# 1: Triggered event, event will be executed when FPGA receives event trigger
# 2: Instant event, event will be executed as soon as possible
#
uint8 execution_condition

# Time in microseconds when the event is triggered.
#
uint64 time_us
