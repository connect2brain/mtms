import time
from threading import Event

import numpy as np

from experiment_interfaces.msg import ExperimentMetadata, IntertrialInterval, \
    Trial, TrialConfig, TrialResult, TriggerConfig
from experiment_interfaces.action import PerformExperiment, PerformTrial
from experiment_interfaces.srv import ValidateTrial, CountValidTrials, PerformExperimentService

from mep_interfaces.msg import Mep

from mtms_device_interfaces.msg import SystemState, SessionState, DeviceState

import rclpy
from rclpy.action import ActionClient, ActionServer
from rclpy.node import Node

from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup


class ExperimentPerformerNode(Node):

    ROS_ACTION_PERFORM_TRIAL = ('/trial/perform', PerformTrial)

    FIRST_TRIAL_TIME_S = 1.0

    def __init__(self):
        super().__init__('experiment_performer_node')

        self.logger = self.get_logger()

        # Enable calling services within actions. See also MultiThreadExecutor.
        self.callback_group = ReentrantCallbackGroup()

        # Create action server for performing experiment.

        self.action_server = ActionServer(
            self,
            PerformExperiment,
            '/experiment/perform',
            self.perform_experiment_action_handler,
            callback_group=self.callback_group,
        )

        # Create service for performing experiment.
        #
        # XXX: Only needed because rosbridge does not support actions, see a more comprehensive comment
        #   in PerformExperiment.srv.

        self.perform_experiment_service = self.create_service(
            PerformExperimentService,
            '/experiment/perform_service',
            self.perform_experiment_service_handler,
            callback_group=self.callback_group,
        )

        # Create service for counting valid trials in an experiment.

        self.count_valid_trials_service = self.create_service(
            CountValidTrials,
            '/experiment/count_valid_trials',
            self.count_valid_trials_callback,
            callback_group=self.callback_group,
        )

        # Create action client for performing a trial.

        topic, action_type = self.ROS_ACTION_PERFORM_TRIAL

        self.perform_trial_client = ActionClient(self, action_type, topic, callback_group=self.callback_group)
        while not self.perform_trial_client.wait_for_server(timeout_sec=1.0):
            self.get_logger().info('Action {} not available, waiting...'.format(topic))

        # Create service client for validating a trial.

        self.validate_trial_client = self.create_client(ValidateTrial, '/trial/validate', callback_group=self.callback_group)
        while not self.validate_trial_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /trial/validate not available, waiting...')

        # Create subscriber for system state.
        self.system_state_subscriber = self.create_subscription(SystemState, '/mtms_device/system_state', self.handle_system_state, 1)
        self.system_state = None

    # ROS callbacks and callers

    # System state

    def handle_system_state(self, system_state):
        self.system_state = system_state

    def get_device_state(self):
        return self.system_state.device_state.value

    def is_device_started(self):
        return self.get_device_state() == DeviceState.OPERATIONAL

    def get_session_state(self):
        return self.system_state.session_state.value

    def is_session_started(self):
        return self.get_session_state() == SessionState.STARTED

    def get_current_time(self):
        return self.system_state.time

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

    def async_service_call(self, client, request):
        call_service_event = Event()
        response_value = [None]

        def service_call_callback(future):
            nonlocal response_value
            response_value[0] = future.result()
            call_service_event.set()

        service_call_future = client.call_async(request)
        service_call_future.add_done_callback(service_call_callback)

        # Wait for the service call to complete
        call_service_event.wait()

        response = response_value[0]
        return response

    # Action callers

    def async_perform_trial_action(self, trial, earliest_trial_time, wait_for_trigger):
        client = self.perform_trial_client
        goal = PerformTrial.Goal()

        goal.trial = trial
        goal.earliest_trial_time = earliest_trial_time
        goal.wait_for_trigger = wait_for_trigger

        event, result_container = self.async_action_call(client, goal)

        return event, result_container

    def sync_perform_trial_action(self, trial, earliest_trial_time, wait_for_trigger):
        event, result_container = self.async_perform_trial_action(
            trial=trial,
            earliest_trial_time=earliest_trial_time,
            wait_for_trigger=wait_for_trigger,
        )
        event.wait()

        result = self.get_result_from_container(result_container)

        assert result.success, "Performing trial was not successful."

        return result

    # Service callers

    def validate_trial(self, trial):
        request = ValidateTrial.Request()
        request.trial = trial

        response = self.async_service_call(self.validate_trial_client, request)

        assert response.success, "Validating trial was not successful."

        return response.is_trial_valid

    # Performing experiment

    def perform_experiment_action_handler(self, goal_handle):
        request = goal_handle.request

        metadata = request.metadata
        trials = request.trials
        intertrial_interval = request.intertrial_interval
        wait_for_trigger = request.wait_for_trigger
        randomize_trials = request.randomize_trials

        # Use short version of goal ID (2 first bytes as hex) for logging.
        #
        uuid = goal_handle.goal_id.uuid
        goal_id = "{:02x}{:02x}".format(uuid[0], uuid[1])

        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: New goal received: {}.'.format(goal_id, goal_id))

        # XXX: There should probably be an Experiment message so the request wouldn't have
        #   to be passed directly.
        self.log_experiment_config(
            goal_id=goal_id,
            experiment=request
        )

        valid_trials = self.get_valid_trials(
            goal_id=goal_id,
            trials=trials,
        )

        trial_results = self.perform_experiment(
            goal_id=goal_id,
            metadata=metadata,
            valid_trials=valid_trials,
            intertrial_interval=intertrial_interval,
            wait_for_trigger=wait_for_trigger,
            randomize_trials=randomize_trials,
        )

        # Create and return a Result object.

        result = PerformExperiment.Result()

        goal_handle.succeed()

        result.trial_results = trial_results

        self.logger.info('{}: Done.'.format(goal_id))

        return result

    def perform_experiment_service_handler(self, request, response):
        metadata = request.metadata
        trials = request.trials
        intertrial_interval = request.intertrial_interval
        wait_for_trigger = request.wait_for_trigger
        randomize_trials = request.randomize_trials

        # HACK: Service calls are not assigned an ID by ROS, therefore assign a constant ID here.
        #   It is used only as the prefix for the log messages.
        #
        goal_id = "abcd"

        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: New goal received: {}.'.format(goal_id, goal_id))

        # XXX: There should probably be an Experiment message so the request wouldn't have
        #   to be passed directly.
        self.log_experiment_config(
            goal_id=goal_id,
            experiment=request
        )

        valid_trials = self.get_valid_trials(
            goal_id=goal_id,
            trials=trials,
        )

        trial_results = self.perform_experiment(
            goal_id=goal_id,
            metadata=metadata,
            valid_trials=valid_trials,
            intertrial_interval=intertrial_interval,
            wait_for_trigger=wait_for_trigger,
            randomize_trials=randomize_trials,
        )

        response.success = True
        response.trial_results = trial_results

        self.logger.info('{}: Done.'.format(goal_id))

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

    def check_goal_feasible(self, goal_id, valid_trials):
        # Check that the mTMS device is started.
        if not self.is_device_started():
            self.logger.info('{}: mTMS device not started, aborting.'.format(goal_id))
            return False

        # Check that the session is started.
        if not self.is_session_started():
            self.logger.info('{}: Session not started on the mTMS device, aborting.'.format(goal_id))
            return False

        # Check that there is at least one valid trial.
        if len(valid_trials) == 0:
            self.logger.info('{}: None of the trials are valid, aborting.'.format(goal_id))
            return False

        return True

    def perform_experiment(self, goal_id, metadata, valid_trials, intertrial_interval, wait_for_trigger, randomize_trials):
        self.check_goal_feasible(
            goal_id=goal_id,
            valid_trials=valid_trials
        )

        num_of_valid_trials = len(valid_trials)
        trial_results = [None] * num_of_valid_trials

        if randomize_trials is True:
            # XXX: Set seed to a constant value for now; think about how to properly make experiments
            #   reproducible but, e.g., not give the same trial sequence to all subjects. Should the
            #   seed be based on the experiment name and the subject name?
            np.random.seed(1234)
            valid_trials = np.random.permutation(valid_trials)

        for i in range(num_of_valid_trials):
            trial = valid_trials[i]

            is_first_trial = i == 0

            trial_result = self.perform_trial(
                goal_id=goal_id,
                trial=trial,
                intertrial_interval=intertrial_interval,
                wait_for_trigger=wait_for_trigger,
                is_first_trial=is_first_trial,
            )

            trial_results[i] = trial_result

            self.logger.info('{}: Successfully performed trial {} / {}.'.format(
                goal_id,
                i + 1,
                num_of_valid_trials
            ))

        return trial_results

    def perform_trial(self, goal_id, trial, intertrial_interval, wait_for_trigger, is_first_trial):
        # Start the first trial at FIRST_TRIAL_TIME_S seconds, otherwise follow intertrial interval.
        if is_first_trial:
            time_to_next_trial = self.FIRST_TRIAL_TIME_S
        else:
            time_to_next_trial = np.random.uniform(
                low=intertrial_interval.min,
                high=intertrial_interval.max,
            )

        earliest_trial_time = self.system_state.time + time_to_next_trial

        result = self.sync_perform_trial_action(
            trial=trial,
            earliest_trial_time=earliest_trial_time,
            wait_for_trigger=wait_for_trigger,
        )

        trial_result = result.trial_result

        return trial_result

def main(args=None):
    rclpy.init(args=args)

    experiment_performer_node = ExperimentPerformerNode()

    # Enable calling services within actions. See also ReentrantCallbackGroup.
    executor = MultiThreadedExecutor()
    try:
        rclpy.spin(experiment_performer_node, executor=executor)
    except KeyboardInterrupt:
        pass

    rclpy.shutdown()


if __name__ == '__main__':
    main()
