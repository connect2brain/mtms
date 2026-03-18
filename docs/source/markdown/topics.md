# mTMS ROS Interface Inventory

This file lists the currently used ROS topics, services, and actions across mTMS software, grouped by domain.

## mTMS Device

### Topics

- `/mtms/device/system_state` - Current mTMS device state (channels, device/session state, errors, time).
- `/mtms/device/session` - Session lifecycle state and session time.
- `/mtms/device/events/feedback/pulse` - Pulse event completion/error feedback.
- `/mtms/device/events/feedback/charge` - Charge event completion/error feedback.
- `/mtms/device/events/feedback/discharge` - Discharge event completion/error feedback.
- `/mtms/device/events/feedback/trigger_out` - Trigger-out event completion/error feedback.
- `/mtms/device/healthcheck` - Health status published by device bridge/simulator.

### Services

- `/mtms/device/start` - Start the mTMS device.
- `/mtms/device/stop` - Stop the mTMS device.
- `/mtms/device/send_settings` - Send stimulation settings to device.
- `/mtms/device/events/request` - Submit pulse/charge/discharge/trigger event requests.
- `/mtms/device/events/trigger` - Trigger all pending events with execution condition set to WAIT_FOR_TRIGGER.
- `/mtms/device/session/start` - Start a stimulation session.
- `/mtms/device/session/stop` - Stop an ongoing stimulation session.

## Stimulation

### Topics

- `/mtms/stimulation/allowed` - Enable/disable stimulation permission.
- `/mtms/trigger_out/allowed` - Enable/disable trigger-out permission.

### Services

- `/mtms/stimulation/get_allowed` - Query whether stimulation is currently allowed.

## Neuronavigation

### Topics

- `/neuronavigation/navigate` - Boolean navigation mode state.  XXX: Seems to be unused.
- `/neuronavigation/optitrack_poses` - Tracked tool/coil poses from OptiTrack bridge.
- `/neuronavigation/coil_target` - Selected coil target updates for neuronavigation.
- `/neuronavigation/create_marker` - Marker creation requests (for stimulation/visualization events).
- `/neuronavigation/started` - Whether neuronavigation is started.
- `/neuronavigation/target_mode/enabled` - Whether target mode is enabled.
- `/neuronavigation/coil_at_target` - Whether the coil is currently at target.
- `/neuronavigation/coil_pose` - Current coil pose published to neuronavigation tools.
- `/neuronavigation/coil_mesh` - Coil mesh geometry for visualization.
- `/neuronavigation/focus` - Current focus pose for visualization and interaction.

### Services

- `/neuronavigation/visualize/targets` - Visualize targets in neuronavigation tooling.
- `/neuronavigation/open_orientation_dialog` - Request opening target orientation dialog.

## Pedal

### Topics

- `/mtms/pedal/connected` - Foot pedal connection state.
- `/mtms/pedal/left_button/pressed` - Left pedal button press/release state.
- `/mtms/pedal/right_button/pressed` - Right pedal button press/release state.

## EEG and MEP analysis

### Topics

- `/mtms/eeg/raw` - Incoming raw EEG samples used by EEG gatherer.
- `/mtms/mep/healthcheck` - MEP analyzer health status.

### Services

- `/mtms/mep/analyze_service` - Service-based MEP analysis endpoint (MATLAB-compatible fallback).

### Actions

- `/mtms/mep/analyze` - Analyze MEP response for a stimulation event.
- `/mtms/eeg/gather` - Gather EEG samples from a requested time window.

## Experiment and Trial

### Services

- `/mtms/experiment/count_valid_trials` - Count valid trials in an experiment definition.
- `/mtms/experiment/pause` - Pause an ongoing experiment.
- `/mtms/experiment/resume` - Resume a paused experiment.
- `/mtms/experiment/cancel` - Cancel an ongoing experiment.
- `/mtms/trial/validate` - Validate trial feasibility before execution.
- `/mtms/trial/log` - Log trial results and metadata.

### Actions

- `/mtms/experiment/perform` - Run a full experiment consisting of multiple trials.
- `/mtms/trial/set_voltages` - Set per-channel capacitor voltages before performing a trial.
- `/mtms/trial/perform` - Execute one trial (targeting, stimulation, feedback integration).

## Targeting

### Services

- `/mtms/targeting/get_target_voltages` - Compute coil voltages for a stimulation target.
- `/mtms/targeting/get_maximum_intensity` - Compute maximum allowed intensity at a target.
- `/mtms/targeting/approximate_waveform` - Approximate waveform for targeting/waveform planning.
- `/mtms/targeting/estimate_voltage_after_pulse` - Estimate post-pulse coil voltage.

## Waveforms

### Services

- `/mtms/waveforms/get_default` - Fetch default waveform for a channel.
- `/mtms/waveforms/get_multipulse_waveforms` - Build waveforms for multi-target/multipulse trials.
- `/mtms/waveforms/reverse_polarity` - Reverse waveform polarity.

## E-Field

### Services

- `/mtms/efield/initialize` - Initialize e-field model/session.
- `/mtms/efield/get_norm` - Compute e-field norm at target coordinate.
- `/mtms/efield/get_efieldvector` - Compute full e-field vector.
- `/mtms/efield/get_ROIefieldvector` - Compute e-field vector values for ROI.
- `/mtms/efield/get_ROIefieldvectorMax` - Compute max e-field in ROI.
- `/mtms/efield/set_coil` - Set active coil model/configuration for e-field.
- `/mtms/efield/set_dIperdt` - Set dI/dt parameter for e-field calculations.
