from MTMSApi import MTMSApi

from experiment_interfaces.msg import Experiment, IntertrialInterval
from targeting_interfaces.msg import (
    TargetingAlgorithm,
    ElectricTarget,
)
from trial_interfaces.msg import (
    Trial,
)


api = MTMSApi(
    verbose=False,
)

handler = api.get_experiment_handler()


target = ElectricTarget(
    displacement_x=0,  # mm
    displacement_y=0,  # mm
    rotation_angle=45,  # deg
    intensity=10,  # V/m
    algorithm=TargetingAlgorithm(
        value=TargetingAlgorithm.LEAST_SQUARES,
    ),
)

mep_config = {
    "emg_channel": 0,  # EMG channel 0 corresponds to the first EMG channel in the amplifier.
    "mep_time_window_start": 0.020,
    "mep_time_window_end": 0.040,
    "preactivation_check_enabled": True,
    "preactivation_check_time_window_start": -0.040,
    "preactivation_check_time_window_end": -0.020,
    "preactivation_check_voltage_range_limit": 70.0,
}

# Enable both trigger outs, coinciding with the beginning of the trial with zero delay.
trigger_enabled = [True, True]
trigger_delay = [0.0, 0.0]

# If analyze MEP is set to True, the mTMS software will automatically analyze the MEPs and write the analysis results
# as csv into ~/mtms_experiment_logs/.
#
# Note that enabling MEP analysis leads to many ways in which a trial can fail, e.g., if EEG is not available
# or the preactivation check fails.
#
# TODO: MEP analysis is now separate from the trial, so needs a separate call to the API.
analyze_mep = False

# Define the trials.
single_pulse_trial = Trial(
    targets=[target],
    pulse_times_since_trial_start=[0.0],
    trigger_enabled=trigger_enabled,
    trigger_delay=trigger_delay,

    # These are the same for all trials.
    voltage_tolerance_proportion_for_precharging=0.1,  # Do not modify
    use_pulse_width_modulation_approximation=True,  # Do not modify
    recharge_after_trial=True,
    dry_run=False,
)

paired_pulse_trial = Trial(
    targets=[target, target],
    pulse_times_since_trial_start=[0.0, 0.1],
    trigger_enabled=trigger_enabled,
    trigger_delay=trigger_delay,

    # These are the same for all trials.
    voltage_tolerance_proportion_for_precharging=0.1,  # Do not modify
    use_pulse_width_modulation_approximation=True,  # Do not modify
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
intertrial_interval = IntertrialInterval(
    min=3.5,
    max=4.5,
    tolerance=0.1, # do not modify
)

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
    intertrial_interval_min=intertrial_interval.min,
    intertrial_interval_max=intertrial_interval.max,
    intertrial_interval_tolerance=intertrial_interval.tolerance,
    randomize_trials=randomize_trials,
    wait_for_pedal_press=wait_for_pedal_press,
    autopause=autopause,
    autopause_interval=autopause_interval,
    analyze_mep=analyze_mep,
    mep_emg_channel=mep_config["emg_channel"],
    mep_time_window_start=mep_config["mep_time_window_start"],
    mep_time_window_end=mep_config["mep_time_window_end"],
    preactivation_check_enabled=mep_config["preactivation_check_enabled"],
    preactivation_check_time_window_start=mep_config["preactivation_check_time_window_start"],
    preactivation_check_time_window_end=mep_config["preactivation_check_time_window_end"],
    preactivation_check_voltage_range_limit=mep_config["preactivation_check_voltage_range_limit"],
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

# 'results' is a list containing TrialResults objects, each including the actual start time of each trial
# and the results of the MEP analysis if it was enabled. The MEP analysis results also include the raw EMG buffer
# for custom analysis. See TrialResult.msg and Mep.msg for more information.
results = handler.get_trial_results()

api.stop_session()
