import time
from threading import Event

import numpy as np

from experiment_interfaces.msg import Trial, TrialConfig, TrialResult, TriggerConfig
from experiment_interfaces.action import PerformTrial, PerformStimulus

from mtms_device_interfaces.msg import SystemState, SessionState, DeviceState
from mtms_device_interfaces.action import SetVoltages

from mep_interfaces.msg import Mep
from mep_interfaces.action import AnalyzeMep

from targeting_interfaces.srv import GetChannelVoltages

import rclpy
from rclpy.action import ActionClient, ActionServer
from rclpy.node import Node

from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup


class TrialPerformerNode(Node):

    ROS_ACTION_ANALYZE_MEP = ('/mep/analyze', AnalyzeMep)
    ROS_ACTION_PERFORM_STIMULUS = ('/stimulus/perform', PerformStimulus)
    ROS_ACTION_SET_VOLTAGES = ('/mtms_device/set_voltages', SetVoltages)

    # TODO: Channel count hardcoded for now.
    N_CHANNELS = 5

    # Ensure that at least this amount of time is reserved after voltages are set,
    # before performing the stimulus.
    TRIAL_TIME_MARGINAL_S = 0.1

    def __init__(self):
        super().__init__('trial_performer_node')

        self.logger = self.get_logger()
        self.callback_group = ReentrantCallbackGroup()

        # Create action server for performing trial.

        self.action_server = ActionServer(
            self,
            PerformTrial,
            '/trial/perform',
            self.perform_trial_action_handler,
            callback_group=self.callback_group,
        )

        # Create action client for performing stimulus.

        topic, action_type = self.ROS_ACTION_PERFORM_STIMULUS

        self.perform_stimulus_client = ActionClient(self, action_type, topic, callback_group=self.callback_group)
        while not self.perform_stimulus_client.wait_for_server(timeout_sec=1.0):
            self.get_logger().info('Action {} not available, waiting...'.format(topic))

        # Create action client for setting voltages.

        topic, action_type = self.ROS_ACTION_SET_VOLTAGES

        self.set_voltages_client = ActionClient(self, action_type, topic, callback_group=self.callback_group)
        while not self.set_voltages_client.wait_for_server(timeout_sec=1.0):
            self.get_logger().info('Action {} not available, waiting...'.format(topic))

        # Create action client for analyzing MEP.

        topic, action_type = self.ROS_ACTION_ANALYZE_MEP

        self.analyze_mep_client = ActionClient(self, action_type, topic, callback_group=self.callback_group)
        while not self.analyze_mep_client.wait_for_server(timeout_sec=1.0):
            self.get_logger().info('Action {} not available, waiting...'.format(topic))

        # Create service client for targeting.

        self.targeting_client = self.create_client(GetChannelVoltages, '/targeting/get_channel_voltages')
        while not self.targeting_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /targeting/get_channel_voltages not available, waiting...')

        # Create subscriber for system state.

        # Have a queue of only one message so that only the latest system state is ever received.
        self.system_state_subscriber = self.create_subscription(SystemState, '/mtms_device/system_state', self.handle_system_state, 1)
        self.system_state = None

    ## ROS callbacks and callers

    # System state

    def handle_system_state(self, system_state):
        self.system_state = system_state

    def get_device_state(self):
        if self.system_state is None:
            return None

        return self.system_state.device_state.value

    def is_device_started(self):
        return self.get_device_state() == DeviceState.OPERATIONAL

    def get_session_state(self):
        if self.system_state is None:
            return None

        return self.system_state.session_state.value

    def is_session_started(self):
        return self.get_session_state() == SessionState.STARTED

    def get_current_time(self):
        if self.system_state is None:
            return None

        return self.system_state.time

    # Logging

    def log_trial(self, goal_id, trial, trial_time, allow_late, wait_for_trigger):
        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: Trial:'.format(goal_id))

        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}:   Number of stimuli: {}'.format(goal_id, len(trial.stimuli)))
        self.logger.info('{}:   Execution time: {:.3f} s'.format(goal_id, trial_time))
        self.logger.info('{}:   Allow trial to be late: {}'.format(goal_id, allow_late))
        self.logger.info('{}:   Wait for trigger: {}'.format(goal_id, wait_for_trigger))
        self.logger.info('{}:'.format(goal_id))

        config = trial.config

        self.logger.info('{}:   MEP analysis:'.format(goal_id))
        self.logger.info('{}:     - Enabled: {}'.format(goal_id, config.analyze_mep))
        if config.analyze_mep:
            self.logger.info('{}:     - EMG channel: {}'.format(goal_id, config.mep_config.emg_channel))
            # TODO: Potentially log the rest of the MEP config, although it will be logged by the EMG analyzer, as well.

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

    ## Action calls

    def async_set_voltages(self, voltages):
        client = self.set_voltages_client
        goal = SetVoltages.Goal()

        goal.voltages = voltages

        event, result_container = self.async_action_call(client, goal)

        return event, result_container

    def sync_set_voltages(self, voltages):
        event, result_container = self.async_set_voltages(voltages)
        event.wait()

        result = self.get_result_from_container(result_container)

        assert result.success, "Setting voltages failed."

    def async_perform_stimulus(self, stimulus, time, wait_for_trigger):
        client = self.perform_stimulus_client
        goal = PerformStimulus.Goal()

        goal.stimulus = stimulus
        goal.time = time
        goal.wait_for_trigger = wait_for_trigger

        event, result_container = self.async_action_call(client, goal)

        return event, result_container

    def async_analyze_mep(self, mep_config, time):
        client = self.analyze_mep_client
        goal = AnalyzeMep.Goal()

        goal.mep_configuration = mep_config
        goal.time = time

        event, result_container = self.async_action_call(client, goal)

        return event, result_container

    ## Service calls

    # Targeting services

    def get_channel_voltages(self, target, intensity):
        request = GetChannelVoltages.Request()

        request.target = target
        request.intensity = intensity

        response = self.async_service_call(self.targeting_client, request)
        assert response.success, "Invalid displacement, rotation angle, or intensity."

        # XXX: Voltages should be integers to begin with.
        voltages = [int(voltage) for voltage in response.voltages]

        return voltages, response.reversed_polarities

    ## Performing trial

    def perform_trial_action_handler(self, goal_handle):
        request = goal_handle.request

        trial = request.trial
        trial_time = request.trial_time
        allow_late = request.allow_late
        wait_for_trigger = request.wait_for_trigger

        # Use short version of goal ID (2 first bytes as hex) for logging.
        #
        uuid = goal_handle.goal_id.uuid
        goal_id = "{:02x}{:02x}".format(uuid[0], uuid[1])

        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: New goal received: {}.'.format(goal_id, goal_id))

        success, trial_result = self.perform_trial(
            goal_id=goal_id,
            trial=trial,
            trial_time=trial_time,
            allow_late=allow_late,
            wait_for_trigger=wait_for_trigger,
        )

        # Create and return a Result object.

        result = PerformTrial.Result()

        goal_handle.succeed()

        result.trial_result = trial_result
        result.success = success

        self.logger.info('{}: Done.'.format(goal_id))

        return result

    def check_goal_feasible(self, goal_id):
        # Check that the mTMS device is started.
        if not self.is_device_started():
            self.logger.info('{}: mTMS device not started, aborting.'.format(goal_id))
            return False

        # Check that the session is started.
        if not self.is_session_started():
            self.logger.info('{}: Session not started on the mTMS device, aborting.'.format(goal_id))
            return False

        return True

    def attempt_trial(self, goal_id, voltages, trial_time, allow_late, wait_for_trigger, stimulus, config):
        self.sync_set_voltages(voltages)

        # Earliest feasible trial time cannot be less than the current time. Also, take
        # into account the marginal that we want to have after setting voltages.
        earliest_feasible_trial_time = self.get_current_time() + self.TRIAL_TIME_MARGINAL_S

        if not allow_late and earliest_feasible_trial_time > trial_time:
            mep = Mep()
            success = False

            return success, mep

        trial_time = max(trial_time, earliest_feasible_trial_time)

        stimulus_event, stimulus_result_container = self.async_perform_stimulus(
            stimulus=stimulus,
            time=trial_time,
            wait_for_trigger=wait_for_trigger,
        )

        analyze_mep = config.analyze_mep
        mep_config = config.mep_config

        if not analyze_mep:
            mep = Mep()
            mep_success = True
        else:
            mep_event, mep_result_container = self.async_analyze_mep(
                mep_config=mep_config,
                time=trial_time,
            )
            mep_event.wait()

            mep_result = self.get_result_from_container(mep_result_container)

            mep = mep_result.mep
            mep_success = mep_result.success

        stimulus_event.wait()

        stimulus_result = self.get_result_from_container(stimulus_result_container)

        # XXX: StimulusResult is not a very useful abstraction for now.
        #   It also makes variable names clash a bit, like here. However,
        #   maybe there will be more fields in it later on, and it needs to be
        #   passed in some other ROS topics, services, or actions, making it worthwhile.
        stimulus_result = stimulus_result.stimulus_result

        stimulus_success = stimulus_result.success

        # Print info messages if either stimulus or MEP analysis or both failed.
        if not stimulus_success:
            self.logger.info('{}: Performing stimulus failed.'.format(goal_id))

        if not mep_success:
            self.logger.info('{}: MEP analysis failed.'.format(goal_id))

        success = stimulus_success and mep_success

        return success, mep

    def perform_trial(self, goal_id, trial, trial_time, allow_late, wait_for_trigger):
        config = trial.config
        stimuli = trial.stimuli

        # Unused at the moment; take into use once a trial supports more than one stimuli.
        stimulus_times_since_trial_start = trial.stimulus_times_since_trial_start

        analyze_mep = config.analyze_mep

        self.log_trial(
            goal_id=goal_id,
            trial=trial,
            trial_time=trial_time,
            allow_late=allow_late,
            wait_for_trigger=wait_for_trigger,
        )

        feasible = self.check_goal_feasible(goal_id)
        if not feasible:
            trial_result = TrialResult()
            success = False

            return success, trial_result

        num_of_stimuli = len(stimuli)
        assert num_of_stimuli == 1, "Only one stimulus per trial currently supported!"

        stimulus = stimuli[0]

        target = stimulus.target
        intensity = stimulus.intensity

        voltages, _ = self.get_channel_voltages(target, intensity)

        self.logger.info('{}: Performing trial...'.format(goal_id))

        success, mep = self.attempt_trial(
            goal_id=goal_id,
            voltages=voltages,
            trial_time=trial_time,
            allow_late=allow_late,
            wait_for_trigger=wait_for_trigger,
            stimulus=stimulus,
            config=config,
        )

        trial_finish_time = self.get_current_time()

        trial_result = TrialResult()

        trial_result.mep = mep
        trial_result.trial_finish_time = trial_finish_time

        return success, trial_result

def main(args=None):
    rclpy.init(args=args)

    trial_performer_node = TrialPerformerNode()

    # Allow several actions to be executed concurrently.
    #
    executor = MultiThreadedExecutor()
    try:
        rclpy.spin(trial_performer_node, executor=executor)
    except KeyboardInterrupt:
        pass

    rclpy.shutdown()


if __name__ == '__main__':
    main()
