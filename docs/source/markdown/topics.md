# mTMS ROS Interface Inventory

This file lists the currently used ROS topics, services, and actions across mTMS software.

## Topics

### Device State and Feedback

- `/mtms_device/system_state` - Current mTMS device state (channels, device/session state, errors, time).
- `/system/session` - Session lifecycle state and session time.
- `/mtms_device/events/feedback/pulse` - Pulse event completion/error feedback.
- `/mtms_device/events/feedback/charge` - Charge event completion/error feedback.
- `/mtms_device/events/feedback/discharge` - Discharge event completion/error feedback.
- `/mtms_device/events/feedback/trigger_out` - Trigger-out event completion/error feedback.
- `/mtms_device/healthcheck` - Health status published by device bridge/simulator.

### Neuronavigation and Planning

- `/planner/inner/state` - Internal mutable planner state used by planner nodes.
- `/planner/state` - Public planner state used by UI and integrations.
- `/navigation/navigate` - Boolean navigation mode state.
- `/neuronavigation/optitrack_poses` - Tracked tool/coil poses from OptiTrack bridge.
- `/neuronavigation/coil_target` - Selected coil target updates for neuronavigation.
- `/neuronavigation/create_marker` - Marker creation requests (for stimulation/visualization events).
- `/neuronavigation/started` - Whether neuronavigation is started.
- `/neuronavigation/target_mode/enabled` - Whether target mode is enabled.
- `/neuronavigation/coil_at_target` - Whether the coil is currently at target.
- `/neuronavigation/coil_pose` - Current coil pose published to neuronavigation tools.
- `/neuronavigation/coil_mesh` - Coil mesh geometry for visualization.
- `/neuronavigation/focus` - Current focus pose for visualization and interaction.

### EEG, MEP, and Pedal I/O

- `/eeg/raw` - Incoming raw EEG samples used by EEG gatherer.
- `/eeg/healthcheck` - EEG receiver availability/health status.
- `/mep/healthcheck` - MEP analyzer health status.
- `/pedal/connected` - Foot pedal connection state.
- `/pedal/left_button/pressed` - Left pedal button press/release state.
- `/pedal/right_button/pressed` - Right pedal button press/release state.

## Services

### mTMS Device Control

- `/mtms_device/start_device` - Start the mTMS device.
- `/mtms_device/stop_device` - Stop the mTMS device.
- `/mtms_device/send_settings` - Send stimulation settings to device.
- `/mtms_device/allow_stimulation` - Enable/disable stimulation permission.
- `/mtms_device/allow_trigger_out` - Enable/disable trigger-out permission.
- `/mtms_device/events/request` - Submit pulse/charge/discharge/trigger event requests.
- `/mtms_device/trigger` - Request an immediate trigger event from bridge node.
- `/system/session/start` - Start a stimulation session.
- `/system/session/stop` - Stop an ongoing stimulation session.
- `/stimulation/allowed` - Query whether stimulation is currently allowed.

### Experiment and Trial

- `/trial/validate` - Validate trial feasibility before execution.
- `/trial/log` - Log trial results and metadata.
- `/experiment/count_valid_trials` - Count valid trials in an experiment definition.
- `/experiment/pause` - Pause an ongoing experiment.
- `/experiment/resume` - Resume a paused experiment.
- `/experiment/cancel` - Cancel an ongoing experiment.
- `/mep/analyze_service` - Service-based MEP analysis endpoint (MATLAB-compatible fallback).

### Planner Editing Operations

- `/planner/add_target` - Add a target to planner state.
- `/planner/remove_target` - Remove a target from planner state.
- `/planner/rename_target` - Rename a target.
- `/planner/set_target` - Set target properties.
- `/planner/set_target_orientation` - Set target orientation.
- `/planner/change_target_index` - Reorder target index.
- `/planner/add_pulse_sequence` - Add a pulse sequence.
- `/planner/remove_pulse_sequence` - Remove a pulse sequence.
- `/planner/rename_pulse_sequence` - Rename a pulse sequence.
- `/planner/add_pulse_to_pulse_sequence` - Add pulse to a sequence.
- `/planner/remove_pulse` - Remove pulse from a sequence.
- `/planner/set_pulse_intensity` - Set pulse intensity.
- `/planner/set_pulse_isi` - Set pulse inter-stimulus interval.
- `/planner/set_pulse_sequence_intensity` - Set sequence pulse intensities.
- `/planner/set_pulse_sequence_isi` - Set sequence pulse ISIs.
- `/planner/change_pulse_index` - Reorder pulse index inside a sequence.
- `/planner/change_comment` - Update planner comment metadata.
- `/planner/toggle_select_target` - Toggle target selection.
- `/planner/toggle_select_pulse_sequence` - Toggle pulse-sequence selection.
- `/planner/toggle_select_pulse` - Toggle pulse selection.
- `/planner/toggle_visible` - Toggle target visibility.
- `/planner/toggle_visible_pulse` - Toggle pulse visibility.
- `/planner/toggle_navigation` - Toggle navigation mode.
- `/planner/clear_state` - Clear/reset planner state.

### Targeting, Waveform, E-Field, and Neuronavigation Support

- `/targeting/get_target_voltages` - Compute coil voltages for a stimulation target.
- `/targeting/get_maximum_intensity` - Compute maximum allowed intensity at a target.
- `/targeting/approximate_waveform` - Approximate waveform for targeting/waveform planning.
- `/targeting/estimate_voltage_after_pulse` - Estimate post-pulse coil voltage.
- `/waveforms/get_default` - Fetch default waveform for a channel.
- `/waveforms/get_multipulse_waveforms` - Build waveforms for multi-target/multipulse trials.
- `/waveforms/reverse_polarity` - Reverse waveform polarity.
- `/efield/initialize` - Initialize e-field model/session.
- `/efield/get_norm` - Compute e-field norm at target coordinate.
- `/efield/get_efieldvector` - Compute full e-field vector.
- `/efield/get_ROIefieldvector` - Compute e-field vector values for ROI.
- `/efield/get_ROIefieldvectorMax` - Compute max e-field in ROI.
- `/efield/set_coil` - Set active coil model/configuration for e-field.
- `/efield/set_dIperdt` - Set dI/dt parameter for e-field calculations.
- `/neuronavigation/visualize/targets` - Visualize targets in neuronavigation tooling.
- `/neuronavigation/open_orientation_dialog` - Request opening target orientation dialog.

## Actions

### Experiment and Trial Execution

- `/experiment/perform` - Run a full experiment consisting of multiple trials.
- `/trial/perform` - Execute one trial (targeting, stimulation, feedback integration).

### Device and Signal Processing

- `/mtms_device/set_voltages` - Set per-channel capacitor voltages on the device.
- `/mep/analyze` - Analyze MEP response for a stimulation event.
- `/eeg/gather` - Gather EEG samples from a requested time window.
