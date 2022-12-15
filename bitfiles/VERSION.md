## Version history

### 0.4.6
- MOD: Send sync out signal at periodic intervals.
- FIX: Pulse polarity on Tubingen device.

### 0.4.5
- FIX: Updating pulse count only worked for first channel.
- FIX: Default values for settings were in micro- or milliseconds, not ticks.

### 0.4.4
- MOD: Change time unit of 'Maximum pulses per time' setting to ticks
- MOD: Change time unit of 'Experiment startup trigger duration' setting to ticks
- MOD: Send time as ticks in event requests
- MOD: Clean up naming of front panel indicators

### 0.4.3
- MOD: Allow charging one channel while discharging another and vice versa.
- FIX: Charging and discharging checks did not work properly with 6-channel (Aalto) device.

### 0.4.2
- MOD: Change default setting for 'experiment start trigger duration' setting from 1 ms to 100 us.
- FIX: Validation of charge events when channel equals channel count.

### 0.4.1
- MOD: Only allow events on channels with coil memory attached.
- FIX: Event validation did not work for charge, discharge, and signal out event types.
- FIX: 'Board startup' error occasionally happening at startup (tentative fix).
- FIX: IGBT feedback check in startup - startup did not pass on Tubingen device.
- FIX: Discharge controller readings during and after shutdown - they showed nonsensical values occasionally.

### 0.4.0 

- MOD: Change pulse final state and IGBT startup state to 'Hold' instead of 'Alternative hold'.
- ADD: Support Aalto target in the redesigned version of FPGA interface.
- MOD: Only update settings when experiment is started
- ADD: Send trigger out when experiment is started
- MOD: Drop voltages to zero when stopping an experiment to avoid 'carry-over' effect for the voltage.
- ADD: 'Experiment state' variable to front panel.
- FIX: Bug in which starting and stopping the device quickly in succession may not always work properly.
- FIX: Bug in which experiment could be quickly started and stopped in succession without an effect.
- MOD: Improve default settings.
