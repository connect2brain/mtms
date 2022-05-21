# Contains information about the event.

uint16 event_id

# If true, wait for an external event trigger to start the event.
bool wait_for_trigger

# Time in microseconds when the event is triggered.
uint64 time_us

# How much the event is delayed in microseconds.
uint32 delay_us
