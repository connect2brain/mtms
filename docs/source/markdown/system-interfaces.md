# System interfaces

## Services

### Topic: `/system/session/start`
#### Service: `system_interfaces.srv.StartSession`
QoS: ROS2 Default

Start session. Response: Boolean indicating if starting was successful.

    ---
    bool success


### Topic: `/system/session/stop`
#### Service: `system_interfaces.srv.StopSession`
QoS: ROS2 Default

Stop session. Response: Boolean indicating if stopping was successful.


    ---
    bool success
