from MTMSApi import MTMSApi

from mtms_experiment_interfaces.msg import Experiment
from mtms_targeting_interfaces.msg import ElectricTarget
from mtms_trial_interfaces.msg import Trial


api = MTMSApi(
    verbose=False,
)

handler = api.get_experiment_handler()


target = ElectricTarget(
    displacement_x=0,  # mm
    displacement_y=0,  # mm
    rotation_angle=45,  # deg
    intensity=10,  # V/m
    algorithm=ElectricTarget.LEAST_SQUARES,
)

# Enable both trigger outs, coinciding with the beginning of the trial with zero delay.
trigger_enabled = [True, True]
trigger_delay = [0.0, 0.0]

# If analyze MEP is set to True, the mTMS software will automatically analyze the MEPs and write the analysis results
# into ~/mtms_experiment_logs/ as a CSV file.
#
# Note that enabling MEP analysis leads to several ways in which a trial can fail, e.g., if EEG is not available
# or the preactivation check fails.
analyze_mep = True

mep_emg_channel = 0  # EMG channel 0 corresponds to the first EMG channel in the amplifier.
mep_time_window_start = 0.020
mep_time_window_end = 0.040
preactivation_check_enabled = False  # If True, it will typically fail with simulated data. Hence, set it to False in this example.
preactivation_check_time_window_start = -0.040
preactivation_check_time_window_end = -0.020
preactivation_check_voltage_range_limit = 70.0


# Define the trials.
single_pulse_trial = Trial(
    targets=[target],
    pulse_times_since_trial_start=[0.0],
    trigger_enabled=trigger_enabled,
    trigger_delay=trigger_delay,

    # These are the same for all trials.
    voltage_tolerance_proportion_for_precharging=0.1,  # This can be kept as is.
    use_pulse_width_modulation_approximation=True,  # This can be kept as is.
    recharge_after_trial=True,
    dry_run=False,
)

paired_pulse_trial = Trial(
    targets=[target, target],
    pulse_times_since_trial_start=[0.0, 0.1],
    trigger_enabled=trigger_enabled,
    trigger_delay=trigger_delay,

    # These are the same for all trials.
    voltage_tolerance_proportion_for_precharging=0.1,  # This can be kept as is.
    use_pulse_width_modulation_approximation=True,  # This can be kept as is.
    recharge_after_trial=True,
    dry_run=False,
)

print("Validating single pulse trial...")

is_valid = handler.validate_trial(single_pulse_trial)
if not is_valid:
    print("Single pulse trial is invalid.")
    exit()

print("Validating paired pulse trial...")

is_valid = handler.validate_trial(paired_pulse_trial)
if not is_valid:
    print("Paired pulse trial is invalid.")
    exit()

trials = 3 * [single_pulse_trial] + 3 * [paired_pulse_trial]

# Other experiment settings.
intertrial_interval_min = 3.5
intertrial_interval_max = 4.5
intertrial_interval_tolerance = 0.1  # This can be kept as is.

# Note that the randomization in the mTMS software uses a constant seed for reproducibility;
# alternatively, you can randomize the 'trials' list yourself.
randomize_trials = True

wait_for_pedal_press = False
autopause = False
autopause_interval = 0

experiment = Experiment(
    experiment_name="Example experiment",
    subject_name="Example subject",
    trials=trials,
    intertrial_interval_min=intertrial_interval_min,
    intertrial_interval_max=intertrial_interval_max,
    intertrial_interval_tolerance=intertrial_interval_tolerance,
    randomize_trials=randomize_trials,
    wait_for_pedal_press=wait_for_pedal_press,
    autopause=autopause,
    autopause_interval=autopause_interval,
    analyze_mep=analyze_mep,
    mep_emg_channel=mep_emg_channel,
    mep_time_window_start=mep_time_window_start,
    mep_time_window_end=mep_time_window_end,
    preactivation_check_enabled=preactivation_check_enabled,
    preactivation_check_time_window_start=preactivation_check_time_window_start,
    preactivation_check_time_window_end=preactivation_check_time_window_end,
    preactivation_check_voltage_range_limit=preactivation_check_voltage_range_limit,
)

api.start_device()
api.start_session()

handler.perform_experiment(experiment)

while not handler.is_done() and not api.is_interrupted():
    # XXX: Note that this is pretrial feedback, not posttrial feedback. They should be separated in the future.
    feedback = handler.wait_for_feedback()

    # Feedback is None only when the experiment has been aborted. Check that case separately.
    #
    # TODO: Implement feedback in the mTMS software even when the experiment is aborted.
    if feedback is None:
        continue

    handler.print_feedback()

    attempt_number = feedback.attempt_number
    trial_number = feedback.trial_number

    if attempt_number == 3:
        print("Too many failed attempts, pausing experiment after the next attempt.")

        handler.pause_experiment()

        inp = input("Press Enter to resume the experiment.")
        handler.resume_experiment()

    if attempt_number == 4:
        print("Too many failed attempts, aborting experiment after the next attempt.")
        handler.cancel_experiment()

if api.is_interrupted():
    print("Experiment was interrupted, aborting experiment.")
    handler.cancel_experiment()

api.stop_session()
