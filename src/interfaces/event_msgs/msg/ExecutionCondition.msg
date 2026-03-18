# Execution condition determines when the event is executed:

# Executed when the time counter of the mTMS device reaches time_us (measured from the start of session).
#
uint8 TIMED=0

# Executed when the mTMS device receives an external trigger or a '/mtms/device/events/trigger' service request.
#
uint8 WAIT_FOR_TRIGGER=1

# Executed immediately when the mTMS device receives the request.
#
uint8 IMMEDIATE=2

uint8 value
