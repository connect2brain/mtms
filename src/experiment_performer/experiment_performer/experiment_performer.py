import time
from threading import Event, Lock, Thread

import numpy as np
import uuid

from mtms_experiment_interfaces.msg import ExperimentFeedback, ExperimentState
from mtms_experiment_interfaces.srv import CountValidTrials, LogTrial, PerformExperiment

from mtms_trial_interfaces.srv import PerformTrial, CacheTrial
from mtms_trial_interfaces.msg import Trial

from std_msgs.msg import Bool, Empty
from std_srvs.srv import Trigger

from mtms_device_interfaces.msg import SystemState, DeviceState

from mtms_system_interfaces.msg import Session
from mtms_system_interfaces.srv import StartSession, StopSession

from mtms_mep_interfaces.srv import AnalyzeMep
from mtms_targeting_interfaces.srv import GetMaximumIntensity

import rclpy
from rclpy.node import Node

from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup


ANALYZE_MEP_STATUS_TO_REASON = {
    AnalyzeMep.Response.PREACTIVATION_FAILED: 'preactivation check failed',
    AnalyzeMep.Response.LATE: 'late response',
    AnalyzeMep.Response.TIMEOUT: 'analysis timed out',
    AnalyzeMep.Response.SAMPLES_DROPPED: 'samples dropped',
    AnalyzeMep.Response.INVALID_EMG_CHANNEL: 'invalid EMG channel',
}


class ExperimentPerformerNode(Node):

    ROS_SERVICE_PERFORM_TRIAL = ('/mtms/trial/perform', PerformTrial)
    ROS_SERVICE_PREPARE_TRIAL = ('/mtms/trial/prepare', Trigger)

    FIRST_TRIAL_TIME_S = 2.0
    TRIAL_REDO_INTERVAL_S = 3.0
    PREPARE_TO_PERFORM_MARGIN_S = 0.1
    SERVICE_CALL_TIMEOUT_S = 16.0  # Stopping a session can take up to 15 seconds.
    SESSION_STATE_WAIT_TIMEOUT_S = 5.0

    HEARTBEAT_TOPIC = '/mtms/experiment_performer/heartbeat'
    HEARTBEAT_PUBLISH_PERIOD = 0.5

    def __init__(self):
        super().__init__('experiment_performer_node')

        self.logger = self.get_logger()

        # Allow nested service/action calls from long-running callbacks.
        self.callback_group = ReentrantCallbackGroup()

        self.heartbeat_publisher = self.create_publisher(Empty, self.HEARTBEAT_TOPIC, 10)
        self.create_timer(self.HEARTBEAT_PUBLISH_PERIOD, lambda: self.heartbeat_publisher.publish(Empty()))

        # Create service for performing experiment.
        self.perform_experiment_service = self.create_service(
            PerformExperiment,
            '/mtms/experiment/perform',
            self.perform_experiment_service_callback,
            callback_group=self.callback_group,
        )

        self.experiment_state_publisher = self.create_publisher(
            ExperimentState,
            '/mtms/experiment/state',
            10,
        )

        self.experiment_feedback_publisher = self.create_publisher(
            ExperimentFeedback,
            '/mtms/experiment/feedback',
            10,
        )

        # Create service for counting valid trials in an experiment.

        self.count_valid_trials_service = self.create_service(
            CountValidTrials,
            '/mtms/experiment/count_valid_trials',
            self.count_valid_trials_callback,
            callback_group=self.callback_group,
        )

        # Create service for pausing an ongoing experiment.

        self.pause_experiment_service = self.create_service(
            Trigger,
            '/mtms/experiment/pause',
            self.pause_experiment_callback,
            callback_group=self.callback_group,
        )

        # Create service for resuming an ongoing experiment.

        self.resume_experiment_service = self.create_service(
            Trigger,
            '/mtms/experiment/resume',
            self.resume_experiment_callback,
            callback_group=self.callback_group,
        )

        # Create service for canceling an ongoing experiment.

        self.cancel_experiment_service = self.create_service(
            Trigger,
            '/mtms/experiment/cancel',
            self.cancel_experiment_callback,
            callback_group=self.callback_group,
        )

        # Create service client for performing a trial.

        service_name, service_type = self.ROS_SERVICE_PERFORM_TRIAL

        self.perform_trial_client = self.create_client(service_type, service_name, callback_group=self.callback_group)
        while not self.perform_trial_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service {} not available, waiting...'.format(service_name))

        # Create service client for preparing a trial.
        service_name, service_type = self.ROS_SERVICE_PREPARE_TRIAL
        self.prepare_trial_client = self.create_client(service_type, service_name, callback_group=self.callback_group)
        while not self.prepare_trial_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service {} not available, waiting...'.format(service_name))

        # Create service client for warming the cache, also used for validating a trial.
        self.cache_trial_client = self.create_client(CacheTrial, '/mtms/trial/cache', callback_group=self.callback_group)
        while not self.cache_trial_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /mtms/trial/cache not available, waiting...')

        # Create service client for maximum intensity validation (single-pulse validation path).
        self.get_maximum_intensity_client = self.create_client(
            GetMaximumIntensity,
            '/mtms/targeting/get_maximum_intensity',
            callback_group=self.callback_group,
        )
        while not self.get_maximum_intensity_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /mtms/targeting/get_maximum_intensity not available, waiting...')

        # Create service client for analyzing MEP.

        self.analyze_mep_client = self.create_client(AnalyzeMep, '/mtms/mep/analyze', callback_group=self.callback_group)
        while not self.analyze_mep_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /mtms/mep/analyze not available, waiting...')

        # Create service client for logging a trial.

        self.log_trial_client = self.create_client(LogTrial, '/mtms/trial/log', callback_group=self.callback_group)
        while not self.log_trial_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /mtms/trial/log not available, waiting...')

        # Create service client to start a session.

        self.start_session_client = self.create_client(StartSession, '/mtms/device/session/start', callback_group=self.callback_group)
        while not self.start_session_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /mtms/device/session/start not available, waiting...')

        # Create service client to stop a session.

        self.stop_session_client = self.create_client(StopSession, '/mtms/device/session/stop', callback_group=self.callback_group)
        while not self.stop_session_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /mtms/device/session/stop not available, waiting...')

        # Create subscriber for system state.
        self.system_state_subscriber = self.create_subscription(SystemState, '/mtms/device/system_state', self.handle_system_state, 1)
        self.system_state = None

        # Create subscriber for session.
        self.session_subscriber = self.create_subscription(Session, '/mtms/device/session', self.handle_session, 1)
        self.session = None

        # Subscriber for pedal.
        self.pedal_subscriber = self.create_subscription(Bool, "/mtms/pedal/right_button/pressed", self.handle_pedal_pressed, 10)
        self.is_pedal_pressed = False

        # Create a lock so that service and action calls can modify the experiment state concurrently.
        self.experiment_state_lock = Lock()
        self.experiment_state = ExperimentState.NOT_RUNNING
        self.pause_entered_at = None
        self.perform_experiment_thread = None

        self.get_logger().info('ExperimentPerformerNode initialized.')

    # Experiment state

    def get_paused_for_seconds(self):
        if self.pause_entered_at is None:
            return 0.0
        return time.monotonic() - self.pause_entered_at

    def publish_experiment_state(self):
        with self.experiment_state_lock:
            state = self.experiment_state

        state_msg = ExperimentState()
        state_msg.state = state
        state_msg.paused_for_seconds = self.get_paused_for_seconds()
        self.experiment_state_publisher.publish(state_msg)

    def set_experiment_state(self, state):
        with self.experiment_state_lock:
            self.experiment_state = state

        self.pause_entered_at = None
        if state == ExperimentState.PAUSED:
            self.pause_entered_at = time.monotonic()

        self.publish_experiment_state()

    def get_experiment_state(self):
        with self.experiment_state_lock:
            return self.experiment_state

    # ROS callbacks and callers

    # System state

    def handle_system_state(self, system_state):
        self.system_state = system_state

    def get_device_state(self):
        if self.system_state is None:
            return None

        return self.system_state.device_state.value

    def is_device_started(self):
        return self.get_device_state() == DeviceState.OPERATIONAL

    # Session

    def handle_session(self, session):
        self.session = session

    def get_session_state(self):
        if self.session is None:
            return None

        return self.session.state

    def is_session_started(self):
        return self.get_session_state() == Session.STARTED

    def is_session_stopped(self):
        return self.get_session_state() == Session.STOPPED

    def get_current_time(self):
        if self.session is None:
            return None

        return self.session.time

    # Other subscribers

    def handle_pedal_pressed(self, msg):
        state = msg.data

        if state:
            self.logger.info('Pedal pressed.')
            self.is_pedal_pressed = True
        else:
            self.logger.info('Pedal released.')
            self.is_pedal_pressed = False

    # Logging

    def log_experiment_config(self, experiment):
        self.logger.info('Experiment configuration:')

        self.logger.info('Experiment:')
        self.logger.info('  - Experiment name: {}'.format(experiment.experiment_name))
        self.logger.info('  - Subject name: {}'.format(experiment.subject_name))
        self.logger.info('Trials:')
        self.logger.info('  - Total # of trials: {}'.format(len(experiment.trials)))
        # TODO
        self.logger.info('  - # of valid trials: {}'.format(len(experiment.trials)))
        self.logger.info('Intertrial interval:')
        self.logger.info('  - Minimum: {}'.format(experiment.intertrial_interval_min))
        self.logger.info('  - Maximum: {}'.format(experiment.intertrial_interval_max))
        self.logger.info('Preactivation check enabled: {}'.format(experiment.preactivation_check_enabled))
        if experiment.preactivation_check_enabled:
            self.logger.info('  - Preactivation check time window start: {}'.format(experiment.preactivation_check_time_window_start))
            self.logger.info('  - Preactivation check time window end: {}'.format(experiment.preactivation_check_time_window_end))
            self.logger.info('  - Preactivation check voltage range limit: {}'.format(experiment.preactivation_check_voltage_range_limit))

        self.logger.info('MEP analysis enabled: {}'.format(experiment.analyze_mep))
        if experiment.analyze_mep:
            self.logger.info('  - MEP EMG channel: {}'.format(experiment.mep_emg_channel))
            self.logger.info('  - MEP time window start: {}'.format(experiment.mep_time_window_start))
            self.logger.info('  - MEP time window end: {}'.format(experiment.mep_time_window_end))

    ## Asynchronous service callers

    def async_service_call(self, client, request, service_name):
        call_service_event = Event()
        response_value = [None]

        def service_call_callback(future):
            nonlocal response_value
            try:
                response_value[0] = future.result()
            except Exception as error:
                self.logger.error('Service {} call failed: {}'.format(service_name, str(error)))
                response_value[0] = None
            call_service_event.set()

        service_call_future = client.call_async(request)
        service_call_future.add_done_callback(service_call_callback)

        # Wait for the service call to complete.
        completed = call_service_event.wait(timeout=self.SERVICE_CALL_TIMEOUT_S)
        if not completed:
            self.logger.warning(
                'Service {} timed out after {:.1f} s.'.format(
                    service_name,
                    self.SERVICE_CALL_TIMEOUT_S,
                )
            )
            return None

        response = response_value[0]
        return response

    # Perform trial

    def perform_trial(self, trial):
        request = PerformTrial.Request()
        request.trial = trial

        response = self.async_service_call(self.perform_trial_client, request, '/mtms/trial/perform')

        if response is None:
            self.logger.error('Service /mtms/trial/perform did not return a response.')
            return None

        return response

    def prepare_trial(self):
        request = Trigger.Request()
        response = self.async_service_call(self.prepare_trial_client, request, '/mtms/trial/prepare')

        if response is None:
            self.logger.error('Service /mtms/trial/prepare did not return a response.')
            return None

        return response

    # Service callers

    def start_session(self):
        request = StartSession.Request()

        response = self.async_service_call(self.start_session_client, request, '/mtms/device/session/start')

        if response is None:
            return False

        if not response.success:
            self.logger.error('Service /mtms/device/session/start returned success=false.')
            return False

        return True

    def stop_session(self):
        request = StopSession.Request()

        response = self.async_service_call(self.stop_session_client, request, '/mtms/device/session/stop')

        if response is None:
            return False

        if not response.success:
            self.logger.error('Service /mtms/device/session/stop returned success=false.')
            return False

        return True

    def cache_trial(self, trial):
        request = CacheTrial.Request()
        request.trial = trial

        response = self.async_service_call(self.cache_trial_client, request, '/mtms/trial/cache')

        if response is None:
            self.logger.error('Service /mtms/trial/cache did not return a response.')
            return None

        success = response.success
        return success

    def get_maximum_intensity(self, displacement_x, displacement_y, rotation_angle, algorithm):
        request = GetMaximumIntensity.Request()
        request.displacement_x = displacement_x
        request.displacement_y = displacement_y
        request.rotation_angle = rotation_angle
        request.algorithm = algorithm

        response = self.async_service_call(
            self.get_maximum_intensity_client,
            request,
            '/mtms/targeting/get_maximum_intensity',
        )

        if response is None:
            self.logger.error('Service /mtms/targeting/get_maximum_intensity did not return a response.')
            return None

        if not response.success:
            self.logger.error('Service /mtms/targeting/get_maximum_intensity returned success=false.')
            return None

        return response.maximum_intensity

    def log_trial(self, experiment_name, subject_name, experiment_id, trial, trial_number, num_of_attempts, mep_amplitude, mep_latency):
        request = LogTrial.Request()

        request.experiment_name = experiment_name
        request.subject_name = subject_name
        request.experiment_id = experiment_id
        request.trial = trial
        request.trial_number = trial_number
        request.num_of_attempts = num_of_attempts
        request.mep_amplitude = mep_amplitude
        request.mep_latency = mep_latency

        response = self.async_service_call(self.log_trial_client, request, '/mtms/trial/log')

        if response is None:
            return False

        if not response.success:
            self.logger.error('Service /mtms/trial/log returned success=false.')
            return False

        return True

    def async_analyze_mep(self, experiment):
        call_service_event = Event()
        response_value = [None]

        request = AnalyzeMep.Request()

        request.emg_channel = experiment.mep_emg_channel

        request.mep_time_window_start = experiment.mep_time_window_start
        request.mep_time_window_end = experiment.mep_time_window_end

        request.preactivation_check_enabled = experiment.preactivation_check_enabled
        request.preactivation_check_time_window_start = experiment.preactivation_check_time_window_start
        request.preactivation_check_time_window_end = experiment.preactivation_check_time_window_end
        request.preactivation_check_voltage_range_limit = experiment.preactivation_check_voltage_range_limit

        def service_call_callback(future):
            nonlocal response_value
            try:
                response_value[0] = future.result()
            except Exception as error:
                self.logger.error('Service /mtms/mep/analyze call failed: {}'.format(str(error)))
                response_value[0] = None
            call_service_event.set()

        service_call_future = self.analyze_mep_client.call_async(request)
        service_call_future.add_done_callback(service_call_callback)

        return call_service_event, response_value

    def get_mep_result_from_container(self, result_container):
        return result_container[0]

    # Performing experiment

    def perform_experiment_service_callback(self, request, response):
        if self.get_experiment_state() != ExperimentState.NOT_RUNNING:
            self.logger.error('Could not start experiment: another experiment is already running.')
            response.success = False
            return response

        if self.perform_experiment_thread is not None and self.perform_experiment_thread.is_alive():
            self.logger.error('Could not start experiment: perform thread is already active.')
            response.success = False
            return response

        experiment_id = uuid.uuid4().hex
        experiment = request.experiment
        self.perform_experiment_thread = Thread(
            target=self.perform_experiment_thread_target,
            args=(experiment_id, experiment),
            daemon=True,
        )
        self.perform_experiment_thread.start()

        # Return immediately; updates are provided via /mtms/experiment/feedback.
        response.success = True
        return response

    def perform_experiment_thread_target(self, experiment_id, experiment):
        valid_trials = []
        success = False

        self.logger.info('New experiment request.')

        self.log_experiment_config(
            experiment=experiment
        )

        try:
            success, valid_trials = self.perform_experiment(
                experiment_id=experiment_id,
                experiment=experiment,
            )
        except Exception as error:
            self.logger.error('Experiment thread failed: {}'.format(str(error)))

        # Publish final trial progress first.
        final_trial = Trial()
        if len(valid_trials) > 0:
            final_trial = valid_trials[-1]

        self.publish_feedback(
            trial=final_trial,
            num_of_attempts=0,
            trial_number=len(valid_trials),
            total_trials=len(valid_trials),
        )

        # Mark experiment as finished so pause/cancel calls are rejected after completion.
        self.set_experiment_state(ExperimentState.NOT_RUNNING)

        self.logger.info('Experiment done (success={}).'.format(success))

    def pause_experiment_callback(self, request, response):
        if self.get_experiment_state() == ExperimentState.NOT_RUNNING:
            self.logger.error('Trying to pause while experiment not running.')

            response.success = False
            return response

        self.logger.info('Pausing experiment after current trial attempt.')

        self.set_experiment_state(ExperimentState.PAUSE_REQUESTED)

        response.success = True
        return response

    def resume_experiment_callback(self, request, response):
        if self.get_experiment_state() == ExperimentState.NOT_RUNNING:
            self.logger.error('Trying to resume while experiment not running.')

            response.success = False
            return response

        self.logger.info('Resuming experiment.')

        self.set_experiment_state(ExperimentState.RUNNING)

        response.success = True
        return response

    def cancel_experiment_callback(self, request, response):
        if self.get_experiment_state() == ExperimentState.NOT_RUNNING:
            self.logger.error('Trying to cancel while experiment not running.')

            response.success = False
            return response

        self.logger.info('Canceling experiment after current trial attempt.')

        self.set_experiment_state(ExperimentState.CANCEL_REQUESTED)

        response.success = True
        return response

    def is_single_pulse_trial_valid(self, trial):
        targets = trial.targets
        if len(targets) != 1:
            return False

        target = targets[0]
        max_intensity = self.get_maximum_intensity(
            displacement_x=target.displacement_x,
            displacement_y=target.displacement_y,
            rotation_angle=target.rotation_angle,
            algorithm=target.algorithm,
        )
        if max_intensity is None:
            return False

        return target.intensity <= max_intensity

    def get_valid_trials(self, trials):
        valid_trials = []
        for trial in trials:
            targets = trial.targets
            if len(targets) == 1:
                if not self.is_single_pulse_trial_valid(trial):
                    continue
            else:
                if not self.cache_trial(trial):
                    continue

            valid_trials.append(trial)

        self.logger.info('{}/{} of the trials are valid.'.format(
            len(valid_trials),
            len(trials),
        ))
        return valid_trials

    def count_valid_trials_callback(self, request, response):
        trials = request.trials

        num_of_trials = len(trials)
        self.logger.info('Received a request for counting valid trials out of {} trials'.format(
            num_of_trials
        ))

        valid_trials = self.get_valid_trials(trials)

        num_of_valid_trials = len(valid_trials)

        response.num_of_valid_trials = num_of_valid_trials
        response.success = True

        return response

    def check_experiment_feasible(self, valid_trials):
        # Check that the mTMS device is started.
        if not self.is_device_started():
            self.logger.info('mTMS device not started, aborting.')
            return False

        # Check that there is at least one valid trial.
        if len(valid_trials) == 0:
            self.logger.info('None of the trials are valid, aborting.')
            return False

        return True

    def get_time_to_next_trial(self, num_of_attempts, is_first_trial, intertrial_interval_min, intertrial_interval_max):
        if num_of_attempts > 1:
            return self.TRIAL_REDO_INTERVAL_S

        # Start the first trial at FIRST_TRIAL_TIME_S seconds, otherwise follow intertrial interval.
        if is_first_trial:
            return self.FIRST_TRIAL_TIME_S
        else:
            return np.random.uniform(
                low=intertrial_interval_min,
                high=intertrial_interval_max,
            )

    def publish_feedback(self, trial, num_of_attempts, trial_number, total_trials):
        feedback_msg = ExperimentFeedback()
        feedback_msg.trial = trial
        feedback_msg.attempt_number = num_of_attempts
        feedback_msg.trial_number = trial_number
        feedback_msg.total_trials = total_trials

        self.experiment_feedback_publisher.publish(feedback_msg)

    def initialize_session(self):

        # If session is already started, stop it first.
        if self.is_session_started():
            self.logger.info('Session already started on the mTMS device, stopping...')
            session_stopped = self.stop_session()
            if not session_stopped:
                self.logger.error('Could not stop existing session before starting a new one.')
                return False

            if not self.wait_for_session_stop():
                return False

        self.logger.info('Starting session.')
        session_started = self.start_session()
        if not session_started:
            self.logger.error('Could not start session.')
            return False

        if not self.wait_for_session_start():
            return False

        return True

    def finalize_session(self):
        self.set_experiment_state(ExperimentState.STOPPING)

        self.logger.info('Stopping session...')
        session_stopped = self.stop_session()
        if not session_stopped:
            self.logger.warning(
                'Stop session call did not complete; continuing shutdown without waiting for session state.'
            )
            return

        if not self.wait_for_session_stop():
            self.logger.warning('Session did not reach STOPPED state before timeout.')

    def wait_for_session_start(self):
        deadline = time.monotonic() + self.SESSION_STATE_WAIT_TIMEOUT_S
        while not self.is_session_started():
            if time.monotonic() >= deadline:
                self.logger.warning(
                    'Timed out after {:.1f} s while waiting for session state STARTED.'
                    .format(self.SESSION_STATE_WAIT_TIMEOUT_S)
                )
                return False

            time.sleep(0.1)

        return True

    def wait_for_session_stop(self):
        deadline = time.monotonic() + self.SESSION_STATE_WAIT_TIMEOUT_S
        while not self.is_session_stopped():
            if time.monotonic() >= deadline:
                self.logger.warning(
                    'Timed out after {:.1f} s while waiting for session state STOPPED.'
                    .format(self.SESSION_STATE_WAIT_TIMEOUT_S)
                )
                return False

            time.sleep(0.1)

        return True

    def perform_experiment(self, experiment_id, experiment):

        experiment_name = experiment.experiment_name
        subject_name = experiment.subject_name
        trials = experiment.trials
        intertrial_interval_min = experiment.intertrial_interval_min
        intertrial_interval_max = experiment.intertrial_interval_max
        wait_for_pedal_press = experiment.wait_for_pedal_press
        randomize_trials = experiment.randomize_trials
        autopause = experiment.autopause
        autopause_interval = experiment.autopause_interval
        analyze_mep = experiment.analyze_mep

        valid_trials = self.get_valid_trials(trials)

        # Initialize experiment state
        self.set_experiment_state(ExperimentState.RUNNING)

        # Check that the experiment is feasible
        feasible = self.check_experiment_feasible(
            valid_trials=valid_trials
        )
        if not feasible:
            success = False
            return success, valid_trials

        num_of_valid_trials = len(valid_trials)

        if randomize_trials is True:
            # XXX: Set seed to a constant value for now; think about how to properly make experiments
            #   reproducible but, e.g., not give the same trial sequence to all subjects. Should the
            #   seed be based on the experiment name and the subject name?
            np.random.seed(1234)
            valid_trials = np.random.permutation(valid_trials)

        if not self.initialize_session():
            return False, valid_trials

        # Loop over trials and perform each.
        success = True
        last_resume_time = self.get_current_time()

        i = 0
        num_of_attempts = 0
        while i < num_of_valid_trials:
            num_of_attempts += 1

            trial = valid_trials[i]
            trial_number = i + 1

            # Check if autopause interval has passed.
            if autopause:
                current_time = self.get_current_time()
                if current_time > last_resume_time + autopause_interval:
                    self.set_experiment_state(ExperimentState.PAUSED)

            # Check if pause request has been received.
            if self.get_experiment_state() == ExperimentState.PAUSE_REQUESTED:
                self.set_experiment_state(ExperimentState.PAUSED)

            # Send feedback at the start of a new trial attempt.
            self.publish_feedback(
                trial=trial,
                num_of_attempts=num_of_attempts,
                trial_number=trial_number,
                total_trials=num_of_valid_trials,
            )

            experiment_state = self.get_experiment_state()

            # Check if the experiment was paused, wait until resumed.
            if experiment_state == ExperimentState.PAUSED:
                self.logger.info('Experiment paused...')
                while self.get_experiment_state() == ExperimentState.PAUSED:
                    self.publish_experiment_state()
                    time.sleep(0.1)

                if self.get_experiment_state() == ExperimentState.CANCEL_REQUESTED:
                    self.logger.info('Experiment canceled while paused.')
                    success = False
                    break

                self.logger.info('Experiment resumed.')
                last_resume_time = self.get_current_time()

                # Re-send feedback after pausing has been finished.
                #
                # XXX: Is this needed? If it is, please document why.
                self.publish_feedback(
                    trial=trial,
                    num_of_attempts=num_of_attempts,
                    trial_number=trial_number,
                    total_trials=num_of_valid_trials,
                )

            # Check if the experiment was canceled.
            if self.get_experiment_state() == ExperimentState.CANCEL_REQUESTED:
                self.logger.info('Experiment canceled.')
                success = False
                break

            # The starting time of the first trial is handled differently, hence the check.
            is_first_trial = i == 0

            if wait_for_pedal_press:
                self.logger.info('Waiting for a pedal press...')

                # Wait for a pedal press.
                while not (self.is_pedal_pressed or self.get_experiment_state() == ExperimentState.CANCEL_REQUESTED):
                    time.sleep(0.1)

                # Check if the experiment was canceled.
                if self.get_experiment_state() == ExperimentState.CANCEL_REQUESTED:
                    self.logger.info('Experiment canceled while waiting for pedal press.')
                    success = False
                    break

                time_to_next_trial = 0.0
            else:
                time_to_next_trial = self.get_time_to_next_trial(
                    num_of_attempts=num_of_attempts,
                    is_first_trial=is_first_trial,
                    intertrial_interval_min=intertrial_interval_min,
                    intertrial_interval_max=intertrial_interval_max,
                )

            # Determine the designated time of the next trial.
            designated_trial_time = self.get_current_time() + time_to_next_trial

            self.logger.info('Performing trial {} / {}, attempt number {}'.format(
                i + 1,
                num_of_valid_trials,
                num_of_attempts,
            ))

            trial.start_time = designated_trial_time

            # Cache the trial.
            cache_ok = self.cache_trial(trial)
            if cache_ok is None:
                self.logger.info('Cache trial service did not return a response, attempting again in {} seconds.'.format(
                    self.TRIAL_REDO_INTERVAL_S,
                ))
                continue

            if not cache_ok:
                # XXX: It can happen at least in theory that a single-pulse trial passes the previous validity check,
                #   but the waveform approximation (done during caching) fails. This is because the validity check uses
                #   only the known maximum intensity for the pulse to have a higher speed, and does not perform the full
                #   waveform approximation. If this happens, skip to the next trial instead of retrying.
                self.logger.info('Trial is invalid, skipping.')
                i += 1
                num_of_attempts = 0
                continue

            # Prepare the trial.
            prepare_response = self.prepare_trial()
            if prepare_response is None:
                self.logger.info('Prepare trial service did not return a response, attempting again in {} seconds.'.format(
                    self.TRIAL_REDO_INTERVAL_S,
                ))
                continue

            if not prepare_response.success:
                self.logger.info('Preparing trial was not successful, attempting again in {} seconds.'.format(
                    self.TRIAL_REDO_INTERVAL_S,
                ))
                time.sleep(0.1)
                continue

            # Determine the earliest possible start time for the trial.
            earliest_possible_trial_time = self.get_current_time() + self.PREPARE_TO_PERFORM_MARGIN_S
            trial.start_time = max(designated_trial_time, earliest_possible_trial_time)

            # Analyze MEP.
            mep_event = None
            mep_result_container = None
            if analyze_mep:
                # Fire off MEP analysis request before triggering the stimulation events,
                # so the analyzer can start waiting for the upcoming data.
                mep_event, mep_result_container = self.async_analyze_mep(experiment)

            # Perform the trial.
            perform_response = self.perform_trial(
                trial=trial,
            )
            if perform_response is None:
                self.logger.info('Trial service did not return a response, attempting again in {} seconds.'.format(
                    self.TRIAL_REDO_INTERVAL_S,
                ))
                continue

            if not perform_response.success:
                self.logger.info('Performing trial was not successful, attempting again in {} seconds.'.format(
                    self.TRIAL_REDO_INTERVAL_S,
                ))

                # Add a delay to allow other ROS service calls to run. XXX: Is this needed?
                time.sleep(0.1)

                continue

            mep_amplitude = 0.0
            mep_latency = 0.0
            if analyze_mep:
                self.logger.info('Analyzing MEP...')
                if mep_event is None or mep_result_container is None:
                    self.logger.warning('MEP analysis not started, retrying in {} seconds.'.format(
                        self.TRIAL_REDO_INTERVAL_S,
                    ))
                    continue

                completed = mep_event.wait(timeout=self.SERVICE_CALL_TIMEOUT_S)
                if not completed:
                    self.logger.warning(
                        'Service /mtms/mep/analyze timed out after {:.1f} s.'.format(
                            self.SERVICE_CALL_TIMEOUT_S,
                        )
                    )
                    self.logger.warning('MEP analysis timed out, retrying in {} seconds.'.format(
                        self.TRIAL_REDO_INTERVAL_S,
                    ))
                    continue

                mep_result = self.get_mep_result_from_container(mep_result_container)

                if mep_result is None:
                    self.logger.warning('MEP analysis result is None, retrying in {} seconds.'.format(
                        self.TRIAL_REDO_INTERVAL_S,
                    ))
                    continue

                mep_amplitude = mep_result.amplitude
                mep_latency = mep_result.latency

                mep_ok = (mep_result.status == AnalyzeMep.Response.NO_ERROR)
                if not mep_ok:
                    failure_reason = ANALYZE_MEP_STATUS_TO_REASON.get(mep_result.status, 'unknown failure status')
                    self.logger.warning('MEP analysis failed (reason={}), attempting again in {} seconds.'.format(
                        failure_reason,
                        self.TRIAL_REDO_INTERVAL_S,
                    ))
                    continue

            self.logger.info('Successfully performed trial {} / {}.'.format(
                i + 1,
                num_of_valid_trials
            ))

            trial_logged = self.log_trial(
                experiment_name=experiment_name,
                subject_name=subject_name,
                experiment_id=experiment_id,
                trial=trial,
                trial_number=trial_number,
                num_of_attempts=num_of_attempts,
                mep_amplitude=mep_amplitude,
                mep_latency=mep_latency,
            )
            if not trial_logged:
                self.logger.error(
                    'Trial {} / {} was performed, but logging did not complete; aborting experiment.'.format(
                        i + 1,
                        num_of_valid_trials,
                    )
                )
                break

            i += 1
            num_of_attempts = 0

            # Add a delay to allow other ROS service calls to run. XXX: Is this needed?
            time.sleep(0.1)

        self.finalize_session()

        return success, valid_trials

def main(args=None):
    rclpy.init(args=args)

    experiment_performer_node = ExperimentPerformerNode()

    # Enable nested calls from long-running callbacks. See also ReentrantCallbackGroup.
    executor = MultiThreadedExecutor()
    try:
        rclpy.spin(experiment_performer_node, executor=executor)
    except KeyboardInterrupt:
        pass

    rclpy.shutdown()


if __name__ == '__main__':
    main()
