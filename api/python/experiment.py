import time
from datetime import datetime

import numpy as np
from pytimedinput import timedKey

from event_interfaces.msg import ExecutionCondition


class Color:
   PURPLE = '\033[95m'
   CYAN = '\033[96m'
   DARKCYAN = '\033[36m'
   BLUE = '\033[94m'
   GREEN = '\033[92m'
   YELLOW = '\033[93m'
   RED = '\033[91m'
   BOLD = '\033[1m'
   UNDERLINE = '\033[4m'
   END = '\033[0m'


class Experiment:
    def __init__(self, experiment_name, test_experiment, api):
        self.api = api
        self.test_experiment = test_experiment
        self.time_of_experiment = datetime.now()
        self.event_log = open("experiment_{}_{}.csv".format(
            experiment_name,
            self.time_of_experiment.strftime('%Y-%m-%d_%H%M%S')), "w"
        )
        self.trials = []
        self.total_duration_of_pauses = 0.0

        np.random.seed(1)

    def configure_mep_analysis(self, enabled, mep_configuration):
        self.analyze_mep_enabled = enabled
        self.mep_configuration = mep_configuration

    def add_trials(self, actions, condition):
        self.trials += [
            {
                'time': None,
                'condition': condition,
                'actions': action,
            } for action in actions
        ]
        self.num_of_trials = len(self.trials)

    def permute_trials(self):
        self.trials = np.random.permutation(self.trials)

    def add_trial_times(self, isi_low, isi_high, first_trial_time=5.0):
        time = first_trial_time
        for i in range(self.num_of_trials):
            self.trials[i]['time'] = time
            time += np.random.uniform(isi_low, isi_high)

    def execute_pulse(self, condition, time, params):
        x = int(params['x'])
        y = int(params['y'])
        angle = int(params['angle'])
        intensity = int(params['intensity'])
        delta_time = params['delta_time']

        # Cap intensity at 5 V/m when testing the experiment.
        if self.test_experiment is True:
            print("Testing the experiment: Capping intensity to 5 V/m")
            print("")

            intensity = min(intensity, 5)

        time_adjusted = time + delta_time

        voltages, reverse_polarities = self.api.get_channel_voltages(
            displacement_x=x,
            displacement_y=y,
            rotation_angle=angle,
            intensity=intensity,
        )

        self.api.send_immediate_charge_or_discharge_to_all_channels(
            target_voltages=voltages,
            wait_for_completion=False,
        )

        self.api.send_timed_default_pulse_to_all_channels(
            reverse_polarities=reverse_polarities,
            time=time_adjusted,
            wait_for_completion=False,
        )

        print("")
        print("Executing pulse at ({}, {}, {}) with intensity {} V/m at time {:.4f} s.".format(x, y, angle, intensity, time_adjusted))

        self.event_log.write("{};{};pulse;{};{}\n".format(time_adjusted, condition, x, y))
        self.event_log.flush()

    def execute_trigger(self, condition, time, params):
        port = params['port']
        delta_time = params['delta_time']

        time_adjusted = time + delta_time

        self.api.send_trigger_out(
            port=port,
            execution_condition=ExecutionCondition.TIMED,
            time=time_adjusted,
            wait_for_completion=False,
        )

        print("")
        print("Executing trigger on port {} at time {:.4f} s.".format(port, time_adjusted))

        self.event_log.write("{};{};trigger\n".format(time_adjusted, condition))
        self.event_log.flush()

    def perform_trial(self, trial):
        condition = trial['condition']
        time = trial['time']
        actions = trial['actions']

        time += self.total_duration_of_pauses

        for action in actions:
            action_type = action['type']
            params = action['params']

            if action_type == 'pulse':
                self.execute_pulse(condition, time, params)

            elif action_type == 'trigger':
                self.execute_trigger(condition, time, params)

        if self.analyze_mep_enabled:
            amplitude, latency = self.analyze_mep(time)
            trial['mep'] = {
                'amplitude': amplitude,
                'latency': latency,
            }

        self.api.wait_until(time + 0.1)

        self.event_log.write("stimulated\n")
        self.event_log.flush()

    def analyze_mep(self, time):
        emg_channel = 1

        print("")
        print("Analyzing MEP on EMG channel {} at time {:.4f} s.".format(emg_channel, time))

        amplitude, latency, errors = self.api.analyze_mep(
            emg_channel=emg_channel,
            time=time,
            mep_configuration=self.mep_configuration,
        )

        if errors.mep_error.value != 0:
            print("WARNING: MEP error occurred.")

        if errors.gather_mep_error.value!= 0:
            print("WARNING: Gather MEP error occurred.")

        if errors.gather_preactivation_error.value!= 0:
            print("WARNING: Gather preactivation error occurred.")

        print("")
        print("Successfully analyzed MEP with amplitude {:.1f} (\u03BCV) and latency {:.1f} (ms).".format(amplitude, 1000 * latency))
        print("")

        return amplitude, latency

    def pause(self):
        self.event_log.write("INTERRUPTED;")
        self.event_log.flush()

        start = time.time()

        ans = None
        while ans not in ['y', 'Y', 'n', 'N']:
            ans = input("Continue? (Y/N) ")

        end = time.time()

        self.total_duration_of_pauses += end - start

        if ans in ['y', 'Y']:
            self.event_log.write("continued\n")
            self.event_log.flush()
            return False

        self.event_log.write("aborted\n")
        self.event_log.flush()
        return True

    def perform(self):

        # Restart session.
        self.api.stop_session()
        self.api.start_session()

        # Do not allow stimulation when testing the experiment.
        if self.test_experiment:
            self.api.allow_stimulation(False)

        try:
            # Cap number of trials to perform to 10 when testing the experiment.
            if self.test_experiment:
                self.num_of_trials = 10
                print("Testing the experiment: Capping # of trials to 10")
                print("")

            for i in range(self.num_of_trials):
                trial = self.trials[i]
                _, timed_out = timedKey("Press any key to pause before the next trial ", allowCharacters="", timeout=0.5)

                if not timed_out:
                    stop_experiment = self.pause()
                    if stop_experiment:
                        break

                print("")
                print("")
                print("{}Trial {}{}".format(Color.BOLD, i + 1, Color.END))
                print("")

                self.perform_trial(trial)

                print("Trial finished.")
        finally:
            self.api.stop_session()
            self.event_log.close()

    def get_valid_trials(self):
        valid_trial_indices = ['mep' in trial and trial['mep']['amplitude'] is not None for trial in self.trials]
        valid_trials = self.trials[valid_trial_indices]

        return valid_trials

    def get_mep_amplitude(self, trial):
        return trial['mep']['amplitude'] if 'mep' in trial else None

    def get_param(self, trial, param):
        return trial['actions'][0]['params'][param]

    def write_to_csv(self):
        with open("experiment_design_{}.csv".format(self.time_of_experiment.strftime('%Y-%m-%d_%H%M%S')), "w") as f:
            for i in range(self.num_of_trials):
                trial = self.trials[i]

                condition = trial['condition']
                time = trial['time']
                actions = trial['actions']

                x = self.get_param(trial, 'x')
                y = self.get_param(trial, 'y')
                mep_amplitude = self.get_mep_amplitude(trial)

                f.write("{};{};{};{};{};{}\n".format(i, time, condition, x, y, mep_amplitude))
