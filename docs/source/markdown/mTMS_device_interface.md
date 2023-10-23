# mTMS device interface



## Services

### Topic: `/mtms_device/allow_stimulation`
#### Service: `mtms_device_interfaces.srv.AllowStimulation`
Allow or disallow stimulation.

Response: Boolean indicating if the service call was successful.

    bool allow_stimulation
    ---
    bool success

### Topic: `/mtms_device/send_settings`
#### Service: `mtms_device_interfaces.srv.SendSettings`

    Settings settings
    ---
    bool success

### Topic: `/mtms_device/start_device`
#### Service: `mtms_device_interfaces.srv.StartDevice`
Start device. Response: Boolean indicating if starting was successful.
    

    ---
    bool success

### Topic: `/mtms_device/stop_device`
#### Service: `mtms_device_interfaces.srv.StopDevice`
Stop device. Response: Boolean indicating if stopping was successful.


    ---
    bool success

### Topic: `/mtms_device/start_session`
#### Service: `mtms_device_interfaces.srv.StartSession`
Start session. Response: Boolean indicating if starting was successful.

    ---
    bool success


### Topic: `/mtms_device/stop_session`
#### Service: `mtms_device_interfaces.srv.StopSession`
Stop session. Response: Boolean indicating if stopping was successful.

    
    ---
    bool success

## Subscribers

### Topic: `/event/send/charge`
#### Message: `event_interfaces.msg.Charge`

    uint8 channel
    uint16 target_voltage
    EventInfo event_info

### Topic: `/event/send/discharge`
#### Message: `event_interfaces.msg.Discharge`

    uint8 channel
    uint16 target_voltage
    EventInfo event_info

### Topic: `/event/send/event_trigger`
#### Message: `event_interfaces.msg.EventTrigger`
Empty message

### Topic: `/event/send/pulse`
#### Message: `event_interfaces.msg.Pulse`

    uint8 channel
    WaveformPiece[] waveform
    EventInfo event_info

### Topic: `/event/send/trigger_out`
#### Message: `event_interfaces.msg.TriggerOut`

    uint8 port # The index of the signal port.
    uint32 duration_us # Duration of the pulse in microseconds.
    EventInfo event_info


## Publishers

### Topic: `/event/pulse_feedback`
#### Message: `event_interfaces.msg.PulseFeedback`
Contains feedback of an event

    uint16 id
    PulseError error

### Topic: `/event/charge_feedback`
#### Message: `event_interfaces.msg.ChargeFeedback`
Contains feedback of an event

    uint16 id
    ChargeError error

### Topic: `/event/discharge_feedback`
#### Message: `event_interfaces.msg.DisChargeFeedback`
Contains feedback of an event

uint16 id
DischargeError error

### Topic: `/event/trigger_out_feedback`
#### Message: `event_interfaces.msg.TriggerOutFeedback`

    uint8 port # The index of the signal port.
    uint32 duration_us # Duration of the pulse in microseconds.
    EventInfo event_info

### Topic: `/node/message`
#### Message: `std_msgs.msg.String`
Standard message string

### Topic: `/mtms_device/system_state`
#### Message: `mtms_device_interfaces.msg.SystemState`
Qos profile:    
 
    ChannelState[] channel_states

    SystemError system_error_cumulative
    SystemError system_error_current
    SystemError system_error_emergency
    
    StartupError startup_error
    
    DeviceState device_state
    SessionState session_state
    
    float64 time

