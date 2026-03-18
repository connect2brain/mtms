import time
from threading import Event, Lock, Thread

import numpy as np

from experiment_interfaces.msg import ExperimentState, ExperimentFeedback
from experiment_interfaces.srv import CountValidTrials, PauseExperiment, ResumeExperiment, CancelExperiment, LogTrial, PerformExperiment

from mtms_trial_interfaces.action import PerformTrial
from mtms_trial_interfaces.srv import ValidateTrial
from mtms_trial_interfaces.msg import TrialTiming, TrialConfig, Trial

from std_msgs.msg import Bool

from mtms_device_interfaces.msg import SystemState, DeviceState

from system_interfaces.msg import Session, SessionState
from system_interfaces.srv import StartSession, StopSession

import rclpy
from rclpy.action import ActionClient
from rclpy.node import Node

from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup


class ExperimentPerformerNode(Node):

    ROS_ACTION_PERFORM_TRIAL = ('/mtms/trial/perform', PerformTrial)

    FIRST_TRIAL_TIME_S = 2.0
    TRIAL_REDO_INTERVAL_S = 3.0
    SERVICE_CALL_TIMEOUT_S = 2.0
    SESSION_STATE_WAIT_TIMEOUT_S = 5.0

    def __init__(self):
        super().__init__('experiment_performer_node')

        self.logger = self.get_logger()

        # Allow nested service/action calls from long-running callbacks.
        self.callback_group = ReentrantCallbackGroup()

        # Create service for performing experiment.
        self.perform_experiment_service = self.create_service(
            PerformExperiment,
            '/mtms/experiment/perform',
            self.perform_experiment_service_callback,
            callback_group=self.callback_group,
        )

        # Publish experiment progress updates for UIs.
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
            PauseExperiment,
            '/mtms/experiment/pause',
            self.pause_experiment_callback,
            callback_group=self.callback_group,
        )

        # Create service for resuming an ongoing experiment.

        self.resume_experiment_service = self.create_service(
            ResumeExperiment,
            '/mtms/experiment/resume',
            self.resume_experiment_callback,
            callback_group=self.callback_group,
        )

        # Create service for canceling an ongoing experiment.

        self.cancel_experiment_service = self.create_service(
            CancelExperiment,
            '/mtms/experiment/cancel',
            self.cancel_experiment_callback,
            callback_group=self.callback_group,
        )

        # Create action client for performing a trial.

        topic, action_type = self.ROS_ACTION_PERFORM_TRIAL

        self.perform_trial_client = ActionClient(self, action_type, topic, callback_group=self.callback_group)
        while not self.perform_trial_client.wait_for_server(timeout_sec=1.0):
            self.get_logger().info('Action {} not available, waiting...'.format(topic))

        # Create service client for validating a trial.

        self.validate_trial_client = self.create_client(ValidateTrial, '/mtms/trial/validate', callback_group=self.callback_group)
        while not self.validate_trial_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /mtms/trial/validate not available, waiting...')

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
        self.perform_experiment_thread = None

    # Experiment state

    def set_experiment_state(self, state):
        with self.experiment_state_lock:
            self.experiment_state = state

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

        return self.session.state.value

    def is_session_started(self):
        return self.get_session_state() == SessionState.STARTED

    def is_session_stopped(self):
        return self.get_session_state() == SessionState.STOPPED

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

    def log_experiment_config(self, goal_id, experiment):
        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: Experiment configuration:'.format(goal_id))

        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: Metadata:'.format(goal_id))
        self.logger.info('{}:   - Experiment name: {}'.format(goal_id, experiment.metadata.experiment_name))
        self.logger.info('{}:   - Subject name: {}'.format(goal_id, experiment.metadata.subject_name))
        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: Trials:'.format(goal_id))
        self.logger.info('{}:   - Total # of trials: {}'.format(goal_id, len(experiment.trials)))
        # TODO
        self.logger.info('{}:   - # of valid trials: {}'.format(goal_id, len(experiment.trials)))
        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: Intertrial interval:'.format(goal_id))
        self.logger.info('{}:   - Minimum: {}'.format(goal_id, experiment.intertrial_interval.min))
        self.logger.info('{}:   - Maximum: {}'.format(goal_id, experiment.intertrial_interval.max))
        self.logger.info('{}:'.format(goal_id))

    ## Asynchronous action and service callers

    def async_action_call(self, client, goal_msg):
        send_goal_event = Event()
        get_result_event = Event()

        goal_response = [None]
        action_result = [None]

        # Send goal to ROS action.
        def send_goal_callback(future):
            nonlocal send_goal_event
            nonlocal goal_response
            goal_response[0] = future.result()
            send_goal_event.set()

        send_goal_future = client.send_goal_async(goal_msg)
        send_goal_future.add_done_callback(send_goal_callback)

        # Wait for the goal to be sent
        send_goal_event.wait()

        goal_handle = goal_response[0]
        if not goal_handle.accepted:
            self.get_logger().info('Goal rejected.')
            return None

        # Get result from ROS action.
        def get_result_done_callback(future):
            nonlocal get_result_event
            nonlocal action_result
            result_response = future.result()
            if result_response is None:
                self.get_logger().info('Action result failed.')
                action_result[0] = None
            else:
                action_result[0] = result_response.result
            get_result_event.set()

        get_result_future = goal_handle.get_result_async()
        get_result_future.add_done_callback(get_result_done_callback)

        return get_result_event, action_result

    def get_result_from_container(self, result_container):
        return result_container[0]

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

    # Action callers

    def async_perform_trial_action(self, trial):
        client = self.perform_trial_client
        goal = PerformTrial.Goal()

        goal.trial = trial

        event, result_container = self.async_action_call(client, goal)

        return event, result_container

    def sync_perform_trial_action(self, trial):
        event, result_container = self.async_perform_trial_action(
            trial=trial,
        )
        event.wait()

        result = self.get_result_from_container(result_container)

        return result

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

    def validate_trial(self, trial):
        request = ValidateTrial.Request()
        request.trial = trial

        response = self.async_service_call(self.validate_trial_client, request, '/mtms/trial/validate')

        if response is None:
            self.logger.error('Service /mtms/trial/validate did not return a response.')
            return False

        assert response.success, "Validating trial was not successful."

        return response.is_trial_valid

    def log_trial(self, metadata, trial, trial_number, trial_result, num_of_attempts):
        request = LogTrial.Request()

        request.metadata = metadata
        request.trial = trial
        request.trial_number = trial_number
        request.trial_result = trial_result
        request.num_of_attempts = num_of_attempts

        response = self.async_service_call(self.log_trial_client, request, '/mtms/trial/log')

        if response is None:
            return False

        if not response.success:
            self.logger.error('Service /mtms/trial/log returned success=false.')
            return False

        return True

    # Performing experiment

    def perform_experiment_service_callback(self, request, response):
        if self.get_experiment_state() != ExperimentState.NOT_RUNNING:
            self.logger.error('Could not start experiment: another experiment is already running.')
            response.success = False
            response.trial_results = []
            return response

        if self.perform_experiment_thread is not None and self.perform_experiment_thread.is_alive():
            self.logger.error('Could not start experiment: perform thread is already active.')
            response.success = False
            response.trial_results = []
            return response

        request_id = "{:04x}".format(time.time_ns() & 0xffff)
        experiment = request.experiment
        self.perform_experiment_thread = Thread(
            target=self.perform_experiment_thread_target,
            args=(request_id, experiment),
            daemon=True,
        )
        self.perform_experiment_thread.start()

        # Return immediately; updates are provided via /mtms/experiment/feedback.
        response.success = True
        response.trial_results = []
        return response

    def perform_experiment_thread_target(self, goal_id, experiment):
        valid_trials = []
        success = False
        trial_results = []

        metadata = experiment.metadata
        trials = experiment.trials
        intertrial_interval = experiment.intertrial_interval
        wait_for_pedal_press = experiment.wait_for_pedal_press
        randomize_trials = experiment.randomize_trials
        autopause = experiment.autopause
        autopause_interval = experiment.autopause_interval

        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: New experiment request: {}.'.format(goal_id, goal_id))

        # XXX: There should probably be an Experiment message so the request wouldn't have
        #   to be passed directly.
        self.log_experiment_config(
            goal_id=goal_id,
            experiment=experiment
        )

        try:
            valid_trials = self.get_valid_trials(
                goal_id=goal_id,
                trials=trials,
            )

            success, trial_results = self.perform_experiment(
                goal_id=goal_id,
                metadata=metadata,
                valid_trials=valid_trials,
                intertrial_interval=intertrial_interval,
                wait_for_pedal_press=wait_for_pedal_press,
                randomize_trials=randomize_trials,
                autopause=autopause,
                autopause_interval=autopause_interval,
            )
        except Exception as error:
            self.logger.error('{}: Experiment thread failed: {}'.format(goal_id, str(error)))

        # Mark experiment as finished so pause/cancel calls are rejected after completion.
        self.set_experiment_state(ExperimentState.NOT_RUNNING)

        # Publish final state update so UI can detect completion.
        final_trial = Trial()
        if len(valid_trials) > 0:
            final_trial = valid_trials[-1]

        self.publish_feedback(
            experiment_state=ExperimentState.NOT_RUNNING,
            trial=final_trial,
            num_of_attempts=0,
            trial_number=len(valid_trials),
            total_trials=len(valid_trials),
        )

        self.logger.info('{}: Done (success={}, trial_results={}).'.format(goal_id, success, len(trial_results)))

    def pause_experiment_callback(self, request, response):
        # TODO: This service callback, as well as resuming and canceling an experiment, need to be
        #   more disciplined about the experiment that they affect. Overall, need to think about
        #   how to communicate state within the node: is it enough to use node-wide variables for
        #   the state, as long as only one experiment is going on at a time? Should starting
        #   several experiments at the same time be prevented? Probably these service calls should
        #   also check that there is an ongoing experiment in the first place. Before doing any of
        #   this, these services affect node-wide state and therefore assume only one ongoing
        #   experiment at a time.
        #
        goal_id = "abcd"

        self.logger.info('{}:'.format(goal_id))

        if self.get_experiment_state() == ExperimentState.NOT_RUNNING:
            self.logger.error('{}: Trying to pause while experiment not running.'.format(goal_id))

            response.success = False
            return response

        self.logger.info('{}: Pausing the experiment after the current trial attempt...'.format(goal_id))

        self.set_experiment_state(ExperimentState.PAUSED)

        response.success = True
        return response

    def resume_experiment_callback(self, request, response):
        goal_id = "abcd"

        self.logger.info('{}:'.format(goal_id))

        if self.get_experiment_state() == ExperimentState.NOT_RUNNING:
            self.logger.error('{}: Trying to resume while experiment not running.'.format(goal_id))

            response.success = False
            return response

        self.logger.info('{}: Resuming the experiment.'.format(goal_id))

        self.set_experiment_state(ExperimentState.RUNNING)

        response.success = True
        return response

    def cancel_experiment_callback(self, request, response):
        goal_id = "abcd"

        self.logger.info('{}:'.format(goal_id))

        if self.get_experiment_state() == ExperimentState.NOT_RUNNING:
            self.logger.error('{}: Trying to cancel while experiment not running.'.format(goal_id))

            response.success = False
            return response

        self.logger.info('{}: Canceling the experiment after the current trial attempt...'.format(goal_id))

        self.set_experiment_state(ExperimentState.CANCELED)

        response.success = True
        return response

    def get_valid_trials(self, goal_id, trials):
        valid_trials = []
        for trial in trials:
            if self.validate_trial(trial):
                valid_trials.append(trial)

        self.logger.info('{}: {}/{} of the trials are valid.'.format(
            goal_id,
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

        # XXX: This is not very clean. ValidTrialCounter should probably be another ROS node?
        valid_trials = self.get_valid_trials('0000', trials)

        num_of_valid_trials = len(valid_trials)

        response.num_of_valid_trials = num_of_valid_trials
        response.success = True

        return response

    def check_experiment_feasible(self, goal_id, valid_trials):
        # Check that the mTMS device is started.
        if not self.is_device_started():
            self.logger.info('{}: mTMS device not started, aborting.'.format(goal_id))
            return False

        # Check that there is at least one valid trial.
        if len(valid_trials) == 0:
            self.logger.info('{}: None of the trials are valid, aborting.'.format(goal_id))
            return False

        return True

    def get_time_to_next_trial(self, num_of_attempts, is_first_trial, intertrial_interval):
        if num_of_attempts > 1:
            return self.TRIAL_REDO_INTERVAL_S

        # Start the first trial at FIRST_TRIAL_TIME_S seconds, otherwise follow intertrial interval.
        if is_first_trial:
            return self.FIRST_TRIAL_TIME_S
        else:
            return np.random.uniform(
                low=intertrial_interval.min,
                high=intertrial_interval.max,
            )

    def publish_feedback(self, experiment_state, trial, num_of_attempts, trial_number, total_trials):
        feedback_msg = ExperimentFeedback()

        feedback_msg.experiment_state = ExperimentState(value=experiment_state)
        feedback_msg.trial = trial
        feedback_msg.attempt_number = num_of_attempts
        feedback_msg.trial_number = trial_number
        feedback_msg.total_trials = total_trials

        self.experiment_feedback_publisher.publish(feedback_msg)

    def initialize_session(self, goal_id):

        # If session is already started, stop it first.
        if self.is_session_started():
            self.logger.info('{}: Session already started on the mTMS device, stopping...'.format(goal_id))
            session_stopped = self.stop_session()
            if not session_stopped:
                self.logger.error('{}: Could not stop existing session before starting a new one.'.format(goal_id))
                return False

            if not self.wait_for_session_stop(goal_id):
                return False

        self.logger.info('{}: Starting session.'.format(goal_id))
        session_started = self.start_session()
        if not session_started:
            self.logger.error('{}: Could not start session.'.format(goal_id))
            return False

        if not self.wait_for_session_start(goal_id):
            return False

        return True

    def finalize_session(self, goal_id):
        self.logger.info('{}: Stopping session...'.format(goal_id))
        session_stopped = self.stop_session()
        if not session_stopped:
            self.logger.warning(
                '{}: Stop session call did not complete; continuing shutdown without waiting for session state.'
                .format(goal_id)
            )
            return

        if not self.wait_for_session_stop(goal_id):
            self.logger.warning('{}: Session did not reach STOPPED state before timeout.'.format(goal_id))

    def wait_for_session_start(self, goal_id):
        deadline = time.monotonic() + self.SESSION_STATE_WAIT_TIMEOUT_S
        while not self.is_session_started():
            if time.monotonic() >= deadline:
                self.logger.warning(
                    '{}: Timed out after {:.1f} s while waiting for session state STARTED.'
                    .format(goal_id, self.SESSION_STATE_WAIT_TIMEOUT_S)
                )
                return False

            time.sleep(0.1)

        return True

    def wait_for_session_stop(self, goal_id):
        deadline = time.monotonic() + self.SESSION_STATE_WAIT_TIMEOUT_S
        while not self.is_session_stopped():
            if time.monotonic() >= deadline:
                self.logger.warning(
                    '{}: Timed out after {:.1f} s while waiting for session state STOPPED.'
                    .format(goal_id, self.SESSION_STATE_WAIT_TIMEOUT_S)
                )
                return False

            time.sleep(0.1)

        return True

    def perform_experiment(self, goal_id, metadata, valid_trials, intertrial_interval, wait_for_pedal_press, randomize_trials, autopause, autopause_interval):

        # Initialize experiment state
        self.set_experiment_state(ExperimentState.RUNNING)

        # Check that the experiment is feasible
        feasible = self.check_experiment_feasible(
            goal_id=goal_id,
            valid_trials=valid_trials
        )
        if not feasible:
            trial_results = []
            success = False

            return success, trial_results

        num_of_valid_trials = len(valid_trials)
        trial_results = []

        if randomize_trials is True:
            # XXX: Set seed to a constant value for now; think about how to properly make experiments
            #   reproducible but, e.g., not give the same trial sequence to all subjects. Should the
            #   seed be based on the experiment name and the subject name?
            np.random.seed(1234)
            valid_trials = np.random.permutation(valid_trials)

        if not self.initialize_session(goal_id):
            return False, trial_results

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

            experiment_state = self.get_experiment_state()

            # Send feedback at the start of a new trial attempt.
            self.publish_feedback(
                experiment_state=experiment_state,
                trial=trial,
                num_of_attempts=num_of_attempts,
                trial_number=trial_number,
                total_trials=num_of_valid_trials,
            )

            # Check if the experiment was paused, wait until resumed.
            if experiment_state == ExperimentState.PAUSED:
                self.logger.info('{}: Experiment paused...'.format(goal_id))
                while experiment_state == ExperimentState.PAUSED:
                    time.sleep(0.1)
                    experiment_state = self.get_experiment_state()

                self.logger.info('{}: Experiment resumed.'.format(goal_id))
                last_resume_time = self.get_current_time()

                # Re-send feedback after pausing has been finished.
                #
                # XXX: Is this needed? If it is, please document why.
                self.publish_feedback(
                    experiment_state=experiment_state,
                    trial=trial,
                    num_of_attempts=num_of_attempts,
                    trial_number=trial_number,
                    total_trials=num_of_valid_trials,
                )

            # Check if the experiment was canceled.
            if experiment_state == ExperimentState.CANCELED:
                self.logger.info('{}: Experiment canceled.'.format(goal_id))

                success = False
                break

            # The starting time of the first trial is handled differently, hence the check.
            is_first_trial = i == 0

            if wait_for_pedal_press:
                self.logger.info('{}: Waiting for a pedal press...'.format(goal_id))

                # Wait for a pedal press.
                while not (self.is_pedal_pressed or experiment_state == ExperimentState.CANCELED):
                    time.sleep(0.1)
                    experiment_state = self.get_experiment_state()

                # Check if the experiment was canceled.
                if experiment_state == ExperimentState.CANCELED:
                    self.logger.info('{}: Experiment canceled while waiting for pedal press.'.format(goal_id))

                    success = False
                    break

                time_to_next_trial = 0.0
            else:
                time_to_next_trial = self.get_time_to_next_trial(
                    num_of_attempts=num_of_attempts,
                    is_first_trial=is_first_trial,
                    intertrial_interval=intertrial_interval,
                )

            # Determine the time of the next trial.
            trial_time = self.get_current_time() + time_to_next_trial

            # When performing an (rTMS style) experiment, always allow late trials, as the exact timing of the pulse is unimportant.
            allow_late = True

            timing = TrialTiming(
                desired_start_time=trial_time,
                allow_late=allow_late,
            )

            self.logger.info('{}: Performing trial {} / {}, attempt number {}'.format(
                goal_id,
                i + 1,
                num_of_valid_trials,
                num_of_attempts,
            ))

            use_pulse_width_modulation_approximation = True if len(trial.targets) > 1 else False
            dry_run = False
            recharge_after_trial = True
            voltage_tolerance_proportion_for_precharging = 0.03

            config = TrialConfig(
                use_pulse_width_modulation_approximation=use_pulse_width_modulation_approximation,
                dry_run=dry_run,
                recharge_after_trial=recharge_after_trial,
                voltage_tolerance_proportion_for_precharging=voltage_tolerance_proportion_for_precharging,
            )

            trial.timing = timing
            trial.config = config

            result = self.sync_perform_trial_action(
                trial=trial,
            )
            trial_result = result.trial_result
            success = result.success

            if success:
                self.logger.info('{}: Successfully performed trial {} / {}.'.format(
                    goal_id,
                    i + 1,
                    num_of_valid_trials
                ))

                trial_logged = self.log_trial(
                    metadata=metadata,
                    trial=trial,
                    trial_number=trial_number,
                    trial_result=trial_result,
                    num_of_attempts=num_of_attempts,
                )
                if not trial_logged:
                    self.logger.error(
                        '{}: Trial {} / {} was performed, but logging did not complete; aborting experiment.'.format(
                            goal_id,
                            i + 1,
                            num_of_valid_trials,
                        )
                    )
                    success = False
                    break

                trial_results.append(trial_result)

                i += 1
                num_of_attempts = 0
            else:
                self.logger.info('{}: Trial not successful, attempting again in {} seconds.'.format(
                    goal_id,
                    self.TRIAL_REDO_INTERVAL_S,
                ))

            # Add a delay to allow other ROS service calls to run.
            time.sleep(0.1)

        self.finalize_session(goal_id)

        return success, trial_results

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
