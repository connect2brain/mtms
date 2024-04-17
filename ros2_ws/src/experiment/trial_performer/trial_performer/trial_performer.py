import time
from threading import Event

import rclpy
from rclpy.action import ActionClient, ActionServer
from rclpy.node import Node

from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup

from experiment_interfaces.msg import TrialResult, TrialFeedback
from experiment_interfaces.action import PerformTrial

from mep_interfaces.msg import Mep
from mep_interfaces.action import AnalyzeMep

from mtms_device_interfaces.msg import SystemState, DeviceState
from mtms_device_interfaces.action import SetVoltages

from system_interfaces.msg import Session, SessionState
from targeting_interfaces.srv import GetMultipulseWaveforms, GetDefaultWaveform, GetTargetVoltages, ReversePolarity

from utility_interfaces.srv import GetNextId

from event_interfaces.msg import (
    ExecutionCondition,
    Pulse,
    PulseFeedback,
    TriggerOut,
    TriggerOutFeedback,
    EventInfo,
    ReadyForEventTrigger
)


class TrialPerformerNode(Node):

    ROS_ACTION_ANALYZE_MEP = ('/mep/analyze', AnalyzeMep)
    ROS_ACTION_SET_VOLTAGES = ('/mtms_device/set_voltages', SetVoltages)

    # TODO: Channel count hardcoded for now.
    NUM_OF_CHANNELS = 5

    TRIGGER_DURATION_US = 1000

    # Ensure that at least this amount of time is reserved after voltages are set,
    # before performing the trial.
    TRIAL_TIME_MARGINAL_S = 0.1

    def __init__(self):
        super().__init__('trial_performer_node')

        self.logger = self.get_logger()
        self.callback_group = ReentrantCallbackGroup()

        # Action server for performing trial.

        self.action_server = ActionServer(
            self,
            PerformTrial,
            '/trial/perform',
            self.perform_trial_action_handler,
            callback_group=self.callback_group,
        )

        # Action client for setting voltages.

        topic, action_type = self.ROS_ACTION_SET_VOLTAGES

        self.set_voltages_client = ActionClient(self, action_type, topic, callback_group=self.callback_group)
        while not self.set_voltages_client.wait_for_server(timeout_sec=1.0):
            self.get_logger().info('Action {} not available, waiting...'.format(topic))

        # Action client for analyzing MEP.

        topic, action_type = self.ROS_ACTION_ANALYZE_MEP

        self.analyze_mep_client = ActionClient(self, action_type, topic, callback_group=self.callback_group)
        while not self.analyze_mep_client.wait_for_server(timeout_sec=1.0):
            self.get_logger().info('Action {} not available, waiting...'.format(topic))

        # Service client for getting next ID.

        self.get_next_id_client = self.create_client(GetNextId, '/utility/get_next_id', callback_group=self.callback_group)
        while not self.get_next_id_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /utility/get_next_id not available, waiting...')

        # Service client for targeting.

        self.targeting_client = self.create_client(GetTargetVoltages, '/targeting/get_target_voltages', callback_group=self.callback_group)
        while not self.targeting_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /targeting/get_target_voltages not available, waiting...')

        # Service client for reversing polarity.
        self.reverse_polarity_client = self.create_client(ReversePolarity, '/waveforms/reverse_polarity', callback_group=self.callback_group)
        while not self.reverse_polarity_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /waveforms/reverse_polarity not available, waiting...')

        # Service client for getting default waveform.

        self.get_default_waveform_client = self.create_client(GetDefaultWaveform, '/waveforms/get_default')
        while not self.get_default_waveform_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /waveforms/get_default not available, waiting...')

        # Service client for getting multipulse waveforms.

        self.get_multipulse_waveforms_client = self.create_client(GetMultipulseWaveforms, '/waveforms/get_multipulse_waveforms')
        while not self.get_multipulse_waveforms_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Service /waveforms/get_multipulse_waveforms not available, waiting...')

        # Subscriber for system state.

        # Have a queue of only one message so that only the latest system state is ever received.
        self.system_state_subscriber = self.create_subscription(SystemState, '/mtms_device/system_state', self.handle_system_state, 1)
        self.system_state = None

        # Subscriber for session
        self.session_subscriber = self.create_subscription(Session, '/system/session', self.handle_session, 1)
        self.session = None

        # Publishers for pulse and trigger out.
        self.send_pulse_publisher = self.create_publisher(Pulse, '/event/send/pulse', 10, callback_group=self.callback_group)
        self.send_trigger_out_publisher = self.create_publisher(TriggerOut, '/event/send/trigger_out', 10, callback_group=self.callback_group)

        # Subscribers for feedback for pulse and trigger out.
        self.pulse_feedback_subscriber = self.create_subscription(PulseFeedback, '/event/pulse_feedback', self.update_event_feedback, 10, callback_group=self.callback_group)
        self.trigger_out_feedback_subscriber = self.create_subscription(TriggerOutFeedback, '/event/trigger_out_feedback', self.update_event_feedback, 10, callback_group=self.callback_group)

        # Publisher for event trigger readiness.
        self.event_trigger_readiness_publisher = self.create_publisher(ReadyForEventTrigger, '/event/trigger/ready', 10, callback_group=self.callback_group)

        # Publisher for trial feedback.
        self.trial_feedback_publisher = self.create_publisher(TrialFeedback, '/trial/feedback', 10, callback_group=self.callback_group)

        self.event_id = 0

        self.event_feedback = {}

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

    # Events

    def update_event_feedback(self, feedback):
        id = feedback.id
        error = feedback.error

        # TODO: Improve feedback logging, preferably by implementing events as ROS actions.
        self.logger.info('Event {} finished with error code: {}'.format(id, error.value))

        self.event_feedback[id] = feedback

    def get_event_feedback(self, id):
        if id not in self.event_feedback:
            return None

        return self.event_feedback[id]

    def wait_for_events_to_finish(self, ids):
        while True:
            feedbacks = [self.get_event_feedback(id) for id in ids]
            if all([x is not None for x in feedbacks]):
                break

            # XXX: This will cause an extra delay of up to 0.1 seconds. However, the correct
            #   fix would be to implement pulses as ROS actions, which removes the need
            #   for this kind of a check. Aim directly at that, hence settle on this for now.
            time.sleep(0.1)

        return feedbacks

    # Logging

    def log_trial(self, goal_id, trial, timing):
        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: Trial:'.format(goal_id))
        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}:   # of targets: {}'.format(goal_id, len(trial.targets)))

        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}:   Timing:'.format(goal_id))
        if not timing.wait_for_trigger:
            self.logger.info('{}:     Desired start time: {:.3f} s'.format(goal_id, timing.desired_start_time))
            self.logger.info('{}:     Allow trial to be late: {}'.format(goal_id, timing.allow_late))
        else:
            self.logger.info('{}:     Waiting for trigger'.format(goal_id))

        self.logger.info('{}:'.format(goal_id))

        self.logger.info('{}:   MEP analysis:'.format(goal_id))
        self.logger.info('{}:     - Enabled: {}'.format(goal_id, trial.analyze_mep))
        if trial.analyze_mep:
            self.logger.info('{}:     - EMG channel: {}'.format(goal_id, trial.mep_config.emg_channel))
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

        goal.voltages = [int(voltage) for voltage in voltages]

        event, result_container = self.async_action_call(client, goal)

        return event, result_container

    def sync_set_voltages(self, voltages):
        event, result_container = self.async_set_voltages(voltages)
        event.wait()

        result = self.get_result_from_container(result_container)

        assert result.success, "Setting voltages failed."

    def async_analyze_mep(self, mep_config, time):
        client = self.analyze_mep_client
        goal = AnalyzeMep.Goal()

        goal.mep_configuration = mep_config
        goal.time = time

        event, result_container = self.async_action_call(client, goal)

        return event, result_container

    ## Service calls

    # Utility services

    def get_next_id(self):
        request = GetNextId.Request()

        response = self.async_service_call(self.get_next_id_client, request)
        assert response.success, "Getting next ID failed."

        return response.id

    # Pulse and trigger out services

    def trigger_out(self, id, time, execution_condition, port):
        message = TriggerOut()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = execution_condition
        event_info.execution_time = float(time)

        message.event_info = event_info
        message.port = port
        message.duration_us = self.TRIGGER_DURATION_US

        self.send_trigger_out_publisher.publish(message)
        self.event_feedback[id] = None

    def pulse(self, id, time, execution_condition, channel, waveform):
        message = Pulse()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = execution_condition
        event_info.execution_time = float(time)

        message.event_info = event_info
        message.channel = channel
        message.waveform = waveform

        self.send_pulse_publisher.publish(message)
        self.event_feedback[id] = None

    def pulse_for_all_channels(self, waveforms_for_coil_set, time, execution_condition):
        ids = [None] * self.NUM_OF_CHANNELS
        for channel in range(self.NUM_OF_CHANNELS):
            id = self.get_next_id()

            waveform = waveforms_for_coil_set[channel]
            self.pulse(
                id=id,
                time=time,
                channel=channel,
                waveform=waveform,
                execution_condition=execution_condition,
            )
            ids[channel] = id

        return ids

    # Targeting services

    def get_target_voltages(self, target):
        request = GetTargetVoltages.Request()

        request.target = target

        response = self.async_service_call(self.targeting_client, request)
        assert response.success, "Invalid displacement, rotation angle, or intensity."

        voltages = response.voltages
        reversed_polarities = response.reversed_polarities

        return voltages, reversed_polarities

    def reverse_polarity(self, waveform):
        request = ReversePolarity.Request()

        request.waveform = waveform

        response = self.async_service_call(self.reverse_polarity_client, request)
        assert response.success, "Reversing polarity unsuccessful."

        return response.waveform

    # Waveform services

    def get_default_waveform(self, channel):
        request = GetDefaultWaveform.Request()

        request.channel = channel

        response = self.async_service_call(self.get_default_waveform_client, request)
        assert response.success, "Invalid channel."

        waveform = response.waveform

        return waveform

    def get_multipulse_waveforms(self, targets, target_waveforms):
        request = GetMultipulseWaveforms.Request()

        request.targets = targets
        request.target_waveforms = target_waveforms

        response = self.async_service_call(self.get_multipulse_waveforms_client, request)
        assert response.success, "Invalid targets or target waveforms."

        initial_voltages = response.initial_voltages
        approximated_waveforms = response.approximated_waveforms

        return initial_voltages, approximated_waveforms

    def get_singlepulse_waveforms(self, target, target_waveforms):
        initial_voltages, reversed_polarities = self.get_target_voltages(target=target)

        target_waveforms_reversed = target_waveforms.copy()
        for channel in range(self.NUM_OF_CHANNELS):
            if reversed_polarities[channel]:
                target_waveforms_reversed[channel] = self.reverse_polarity(target_waveforms[channel])

        # With only one target, the approximated waveforms are the same as the target waveforms.
        approximated_waveforms = [target_waveforms_reversed]

        return initial_voltages, approximated_waveforms

    ## Performing trial

    def perform_trial_action_handler(self, goal_handle):
        request = goal_handle.request

        trial = request.trial
        timing = request.timing

        # Use short version of goal ID (2 first bytes as hex) for logging.
        #
        uuid = goal_handle.goal_id.uuid
        goal_id = "{:02x}{:02x}".format(uuid[0], uuid[1])

        self.logger.info('{}:'.format(goal_id))
        self.logger.info('{}: New goal received: {}.'.format(goal_id, goal_id))

        success, trial_result = self.perform_trial(
            goal_id=goal_id,
            trial=trial,
            timing=timing,
        )

        # Create and return a Result object.

        result = PerformTrial.Result()

        goal_handle.succeed()

        result.trial_result = trial_result
        result.success = success

        self.logger.info('{}: Done.'.format(goal_id))

        return result

    def check_trial_feasible(self, goal_id):
        # Check that the mTMS device is started.
        if not self.is_device_started():
            self.logger.info('{}: mTMS device not started, aborting.'.format(goal_id))
            return False

        # Check that the session is started.
        if not self.is_session_started():
            self.logger.info('{}: Session not started on the mTMS device, aborting.'.format(goal_id))
            return False

        return True

    def perform_trial(self, goal_id, trial, timing):
        pulse_times_since_trial_start = trial.pulse_times_since_trial_start
        analyze_mep = trial.analyze_mep
        mep_config = trial.mep_config
        triggers = trial.triggers

        desired_start_time = timing.desired_start_time
        allow_late = timing.allow_late
        wait_for_trigger = timing.wait_for_trigger

        self.log_trial(
            goal_id=goal_id,
            trial=trial,
            timing=timing,
        )

        feasible = self.check_trial_feasible(goal_id)
        if not feasible:
            trial_result = TrialResult()
            success = False

            return success, trial_result

        self.logger.info('{}: Performing trial...'.format(goal_id))

        targets = trial.targets

        target_waveforms = [self.get_default_waveform(channel) for channel in range(self.NUM_OF_CHANNELS)]

        # Use PWM approximation if there are multiple targets.
        if len(targets) > 1:
            # TODO: This codepath is not really tested, probably doesn't work properly.
            initial_voltages, approximated_waveforms = self.get_multipulse_waveforms(
                targets=targets,
                target_waveforms=target_waveforms,
            )
        else:
            initial_voltages, approximated_waveforms = self.get_singlepulse_waveforms(
                target=targets[0],
                target_waveforms=target_waveforms,
            )

        self.sync_set_voltages(initial_voltages)

        self.logger.info('{}: Voltages set.'.format(goal_id))

        # Earliest feasible time for the trial cannot be less than the current time. Also, take
        # into account the marginal that we want to have after setting voltages.
        earliest_feasible_time = self.get_current_time() + self.TRIAL_TIME_MARGINAL_S

        if earliest_feasible_time > desired_start_time and not allow_late:
            mep = Mep()
            success = False

            return success, mep

        start_time = max(desired_start_time, earliest_feasible_time)

        # Perform pulses
        for target_idx in range(len(targets)):
            waveforms_for_coil_set = approximated_waveforms[target_idx]
            time = start_time + pulse_times_since_trial_start[target_idx]

            # XXX: Keeping track of the IDs is a bit messy; should use ROS actions instead
            #   to hide the logic.
            pulse_ids = self.pulse_for_all_channels(
                waveforms_for_coil_set=waveforms_for_coil_set,
                time=time,
                execution_condition=ExecutionCondition.TIMED,
            )

        # Perform trigger outs
        num_of_trigger_out_ports = len(triggers)
        trigger_ids = []
        for i in range(num_of_trigger_out_ports):
            if triggers[i].enabled:
                id = self.get_next_id()
                delay = triggers[i].delay
                delayed_time = time + delay
                port = i + 1

                self.trigger_out(
                    id=id,
                    port=port,
                    time=delayed_time,
                    execution_condition=ExecutionCondition.TIMED,
                )
                trigger_ids += [id]

        # Send request for MEP analysis.
        if analyze_mep:
            mep_event, mep_result_container = self.async_analyze_mep(
                mep_config=mep_config,
                time=start_time,
            )

        # TÄLLE JOTAIN
        if wait_for_trigger:
            msg = ReadyForEventTrigger()
            self.event_trigger_readiness_publisher.publish(msg)

            self.logger.info('{}: Waiting for trigger...'.format(goal_id))
        else:
            self.logger.info('{}: Waiting for pulse and trigger out(s) to finish...'.format(goal_id))

        pulse_feedbacks = self.wait_for_events_to_finish(pulse_ids)
        trigger_out_feedbacks = self.wait_for_events_to_finish(trigger_ids)

        # TODO: If there is an error, do something with the error code.
        pulse_success = all([feedback.error.value == 0 for feedback in pulse_feedbacks])
        trigger_out_success = all([feedback.error.value == 0 for feedback in trigger_out_feedbacks])

        # Determine execution time from the first pulse. All pulses should be executed concurrently, so it doesn't matter which one is used.
        execution_time = pulse_feedbacks[0].execution_time

        # Check that all pulses were executed concurrently.
        pulses_executed_concurrently = all([feedback.execution_time == execution_time for feedback in pulse_feedbacks])

        if not pulses_executed_concurrently:
            self.logger.error('{}: Pulses on different channels were not executed concurrently.'.format(goal_id))

        success = pulse_success and trigger_out_success and pulses_executed_concurrently

        self.logger.info('{}: Done! Trial {}.'.format(
            goal_id,
            'was successful' if success else 'failed'
        ))

        if analyze_mep:
            mep_event.wait()

            mep_result = self.get_result_from_container(mep_result_container)

            mep = mep_result.mep
            mep_success = mep_result.success
        else:
            # Return a dummy MEP if MEP analysis was not requested.
            mep = Mep()
            mep_success = True

        # Publish trial feedback.
        feedback = TrialFeedback()

        feedback.success = success
        feedback.execution_time = execution_time

        self.trial_feedback_publisher.publish(feedback)

        self.logger.info('{}: Waiting for trial to be performed.'.format(goal_id))

        if not mep_success:
            self.logger.info('{}: MEP analysis failed.'.format(goal_id))

        success = mep_success

        result = TrialResult()

        result.mep = mep
        result.actual_start_time = execution_time

        return success, result


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
