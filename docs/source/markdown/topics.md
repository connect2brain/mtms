# mTMS ROS Interface Inventory

This file lists the currently used ROS topics, services, and actions across mTMS software.

## Topics

### Device State and Feedback

- `/mtms/device/system_state` - Current mTMS device state (channels, device/session state, errors, time).
- `/mtms/device/session` - Session lifecycle state and session time.
- `/mtms/device/events/feedback/pulse` - Pulse event completion/error feedback.
- `/mtms/device/events/feedback/charge` - Charge event completion/error feedback.
- `/mtms/device/events/feedback/discharge` - Discharge event completion/error feedback.
- `/mtms/device/events/feedback/trigger_out` - Trigger-out event completion/error feedback.
- `/mtms/device/healthcheck` - Health status published by device bridge/simulator.

### Planner
- `/mtms/planner/inner/state` - Internal mutable planner state used by planner nodes.
- `/mtms/planner/state` - Public planner state used by UI and integrations.

### Neuronavigation

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

- `/mtms/eeg/raw` - Incoming raw EEG samples used by EEG gatherer.
- `/mtms/eeg/healthcheck` - EEG receiver availability/health status.
- `/mtms/mep/healthcheck` - MEP analyzer health status.
- `/mtms/pedal/connected` - Foot pedal connection state.
- `/mtms/pedal/left_button/pressed` - Left pedal button press/release state.
- `/mtms/pedal/right_button/pressed` - Right pedal button press/release state.

## Services

### mTMS Device Control

- `/mtms/device/start_device` - Start the mTMS device.
- `/mtms/device/stop_device` - Stop the mTMS device.
- `/mtms/device/send_settings` - Send stimulation settings to device.
- `/mtms/device/allow_stimulation` - Enable/disable stimulation permission.
- `/mtms/device/allow_trigger_out` - Enable/disable trigger-out permission.
- `/mtms/device/events/request` - Submit pulse/charge/discharge/trigger event requests.
- `/mtms/device/trigger` - Request an immediate trigger event from bridge node.
- `/mtms/device/session/start` - Start a stimulation session.
- `/mtms/device/session/stop` - Stop an ongoing stimulation session.
- `/mtms/stimulation/allowed` - Query whether stimulation is currently allowed.

### Experiment and Trial

- `/mtms/trial/validate` - Validate trial feasibility before execution.
- `/mtms/trial/log` - Log trial results and metadata.
- `/mtms/experiment/count_valid_trials` - Count valid trials in an experiment definition.
- `/mtms/experiment/pause` - Pause an ongoing experiment.
- `/mtms/experiment/resume` - Resume a paused experiment.
- `/mtms/experiment/cancel` - Cancel an ongoing experiment.
- `/mtms/mep/analyze_service` - Service-based MEP analysis endpoint (MATLAB-compatible fallback).

### Planner Editing Operations

- `/mtms/planner/add_target` - Add a target to planner state.
- `/mtms/planner/remove_target` - Remove a target from planner state.
- `/mtms/planner/rename_target` - Rename a target.
- `/mtms/planner/set_target` - Set target properties.
- `/mtms/planner/set_target_orientation` - Set target orientation.
- `/mtms/planner/change_target_index` - Reorder target index.
- `/mtms/planner/add_pulse_sequence` - Add a pulse sequence.
- `/mtms/planner/remove_pulse_sequence` - Remove a pulse sequence.
- `/mtms/planner/rename_pulse_sequence` - Rename a pulse sequence.
- `/mtms/planner/add_pulse_to_pulse_sequence` - Add pulse to a sequence.
- `/mtms/planner/remove_pulse` - Remove pulse from a sequence.
- `/mtms/planner/set_pulse_intensity` - Set pulse intensity.
- `/mtms/planner/set_pulse_isi` - Set pulse inter-stimulus interval.
- `/mtms/planner/set_pulse_sequence_intensity` - Set sequence pulse intensities.
- `/mtms/planner/set_pulse_sequence_isi` - Set sequence pulse ISIs.
- `/mtms/planner/change_pulse_index` - Reorder pulse index inside a sequence.
- `/mtms/planner/change_comment` - Update planner comment metadata.
- `/mtms/planner/toggle_select_target` - Toggle target selection.
- `/mtms/planner/toggle_select_pulse_sequence` - Toggle pulse-sequence selection.
- `/mtms/planner/toggle_select_pulse` - Toggle pulse selection.
- `/mtms/planner/toggle_visible` - Toggle target visibility.
- `/mtms/planner/toggle_visible_pulse` - Toggle pulse visibility.
- `/mtms/planner/toggle_navigation` - Toggle navigation mode.
- `/mtms/planner/clear_state` - Clear/reset planner state.

### Targeting, Waveform, E-Field

- `/mtms/targeting/get_target_voltages` - Compute coil voltages for a stimulation target.
- `/mtms/targeting/get_maximum_intensity` - Compute maximum allowed intensity at a target.
- `/mtms/targeting/approximate_waveform` - Approximate waveform for targeting/waveform planning.
- `/mtms/targeting/estimate_voltage_after_pulse` - Estimate post-pulse coil voltage.
- `/mtms/waveforms/get_default` - Fetch default waveform for a channel.
- `/mtms/waveforms/get_multipulse_waveforms` - Build waveforms for multi-target/multipulse trials.
- `/mtms/waveforms/reverse_polarity` - Reverse waveform polarity.
- `/mtms/efield/initialize` - Initialize e-field model/session.
- `/mtms/efield/get_norm` - Compute e-field norm at target coordinate.
- `/mtms/efield/get_efieldvector` - Compute full e-field vector.
- `/mtms/efield/get_ROIefieldvector` - Compute e-field vector values for ROI.
- `/mtms/efield/get_ROIefieldvectorMax` - Compute max e-field in ROI.
- `/mtms/efield/set_coil` - Set active coil model/configuration for e-field.
- `/mtms/efield/set_dIperdt` - Set dI/dt parameter for e-field calculations.

### Neuronavigation

- `/neuronavigation/visualize/targets` - Visualize targets in neuronavigation tooling.
- `/neuronavigation/open_orientation_dialog` - Request opening target orientation dialog.

## Actions

### Experiment and Trial Execution

- `/mtms/experiment/perform` - Run a full experiment consisting of multiple trials.
- `/mtms/trial/perform` - Execute one trial (targeting, stimulation, feedback integration).

### Device and Signal Processing

- `/mtms/device/set_voltages` - Set per-channel capacitor voltages on the device.
- `/mtms/mep/analyze` - Analyze MEP response for a stimulation event.
- `/mtms/eeg/gather` - Gather EEG samples from a requested time window.
