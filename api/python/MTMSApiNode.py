import rclpy
from rclpy.action import ActionClient
from rclpy.callback_groups import ReentrantCallbackGroup
from rclpy.node import Node
from rclpy.qos import QoSProfile, DurabilityPolicy, ReliabilityPolicy, HistoryPolicy

from std_msgs.msg import Bool
from std_srvs.srv import Trigger

from mtms_device_interfaces.msg import SystemState
from mtms_device_interfaces.srv import StartDevice, StopDevice, RequestEvents

from system_interfaces.msg import Session
from system_interfaces.srv import StartSession, StopSession, GetStimulationAllowed

from event_msgs.msg import (
    Charge,
    ChargeFeedback,
    Discharge,
    DischargeFeedback,
    EventInfo,
    Pulse,
    PulseFeedback,
    TriggerOut,
    TriggerOutFeedback,
)

from mep_interfaces.action import AnalyzeMep

from targeting_services.srv import (
    GetTargetVoltages,
    GetMaximumIntensity,
    GetDefaultWaveform,
    ReversePolarity,
    GetMultipulseWaveforms
)

from MTMSApiPrinter import MTMSApiPrinter
from ExperimentHandler import ExperimentHandler


class MTMSApiNode(Node):
    # To mTMS device
    ROS_SERVICE_START_DEVICE = ('/mtms/device/start', StartDevice)
    ROS_SERVICE_STOP_DEVICE = ('/mtms/device/stop', StopDevice)

    ROS_SERVICE_REQUEST_EVENTS = ('/mtms/device/events/request', RequestEvents)
    ROS_SERVICE_TRIGGER_EVENTS = ('/mtms/device/events/trigger', Trigger)

    # To other parts of the system
    ROS_SERVICE_START_SESSION = ('/mtms/device/session/start', StartSession)
    ROS_SERVICE_STOP_SESSION = ('/mtms/device/session/stop', StopSession)

    ROS_SERVICE_GET_TARGET_VOLTAGES = ('/mtms/targeting/get_target_voltages', GetTargetVoltages)
    ROS_SERVICE_GET_MAXIMUM_INTENSITY = ('/mtms/targeting/get_maximum_intensity', GetMaximumIntensity)
    ROS_SERVICE_GET_DEFAULT_WAVEFORM = ('/mtms/waveforms/get_default', GetDefaultWaveform)
    ROS_SERVICE_GET_MULTIPULSE_WAVEFORMS = ('/mtms/waveforms/get_multipulse_waveforms', GetMultipulseWaveforms)
    ROS_SERVICE_REVERSE_POLARITY = ('/mtms/waveforms/reverse_polarity', ReversePolarity)

    ROS_SERVICE_IS_STIMULATION_ALLOWED= ('/mtms/stimulation/get_allowed', GetStimulationAllowed)

    ROS_ACTION_ANALYZE_MEP = ('/mtms/mep/analyze', AnalyzeMep)

    ROS_SERVICES = (
        ROS_SERVICE_START_DEVICE,
        ROS_SERVICE_STOP_DEVICE,
        ROS_SERVICE_START_SESSION,
        ROS_SERVICE_STOP_SESSION,
        ROS_SERVICE_GET_TARGET_VOLTAGES,
        ROS_SERVICE_GET_MAXIMUM_INTENSITY,
        ROS_SERVICE_GET_DEFAULT_WAVEFORM,
        ROS_SERVICE_GET_MULTIPULSE_WAVEFORMS,
        ROS_SERVICE_REVERSE_POLARITY,
        ROS_SERVICE_IS_STIMULATION_ALLOWED,
        ROS_SERVICE_REQUEST_EVENTS,
        ROS_SERVICE_TRIGGER_EVENTS,
    )

    ROS_ACTIONS = (
        ROS_ACTION_ANALYZE_MEP,
    )

    def __init__(self, channel_count, verbose):
        super().__init__('mtms_api_node')
        self.logger = self.get_logger()

        self.verbose = verbose
        self.printer = MTMSApiPrinter(
            channel_count=channel_count,
        )

        self.experiment_handler = ExperimentHandler(self)

        self.system_state = None
        self.session = None

        # Service clients

        self.ros_service_clients = {}

        for topic, service_type in self.ROS_SERVICES:
            client = self.create_client(service_type, topic)
            while not client.wait_for_service(timeout_sec=1.0):
                self.logger.info('Service {} not available, waiting...'.format(topic))

            self.ros_service_clients[topic] = client

        # Action clients

        self.ros_action_clients = {}

        for topic, action_type in self.ROS_ACTIONS:
            # Use ReentrantCallbackGroup to allow multiple actions to be executed simultaneously.
            client = ActionClient(self, action_type, topic, callback_group=ReentrantCallbackGroup())
            while not client.wait_for_server(timeout_sec=1.0):
                self.logger.info('Action {} not available, waiting...'.format(topic))

            self.ros_action_clients[topic] = client

        # Have a queue of only one message so that only the latest system state is ever received.
        self.system_state_subscriber = self.create_subscription(SystemState, '/mtms/device/system_state', self.handle_system_state, 1)

        # Similarly with session.
        self.session_subscriber = self.create_subscription(Session, '/mtms/device/session', self.handle_session, 1)

        self.pulse_feedback_subscriber = self.create_subscription(PulseFeedback, '/mtms/device/events/feedback/pulse', self.handle_pulse_feedback, 10)
        self.charge_feedback_subscriber = self.create_subscription(ChargeFeedback, '/mtms/device/events/feedback/charge', self.handle_charge_feedback, 10)
        self.discharge_feedback_subscriber = self.create_subscription(DischargeFeedback, '/mtms/device/events/feedback/discharge', self.handle_discharge_feedback, 10)
        self.trigger_out_feedback_subscriber = self.create_subscription(TriggerOutFeedback, '/mtms/device/events/feedback/trigger_out', self.handle_trigger_out_feedback, 10)

        # Define a QoS profile with Transient Local durability
        decider_qos = QoSProfile(
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
            depth=1
        )

        self.decider_enabled_subscriber = self.create_subscription(Bool, '/pipeline/decider/enabled', self.handle_decider_enabled, decider_qos)
        self.is_decider_enabled = False

        self.event_feedback = {}

    def call_service(self, client, request):
        self.future = client.call_async(request)
        rclpy.spin_until_future_complete(self, self.future)
        return self.future.result()

    # Starting and stopping

    def start_device(self):
        topic, service_type = self.ROS_SERVICE_START_DEVICE

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        return self.call_service(client, request)

    def stop_device(self):
        topic, service_type = self.ROS_SERVICE_STOP_DEVICE

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        return self.call_service(client, request)

    def start_session(self):
        topic, service_type = self.ROS_SERVICE_START_SESSION

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        return self.call_service(client, request)

    def stop_session(self):
        topic, service_type = self.ROS_SERVICE_STOP_SESSION

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        return self.call_service(client, request)

    # Events

    def trigger_events(self):
        topic, service_type = self.ROS_SERVICE_TRIGGER_EVENTS

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        return self.call_service(client, request)

    def send_pulse(self, id, execution_condition, time, channel, waveform):
        topic, service_type = self.ROS_SERVICE_REQUEST_EVENTS

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = execution_condition
        event_info.execution_time = float(time) if time is not None else 0.0

        pulse = Pulse(
            event_info=event_info,
            channel=channel,
            waveform=waveform,
        )
        request.pulses.append(pulse)

        response = self.call_service(client, request)
        assert response.success, "Failed pulse request."

        self.printer.print_event(
            event_type='Pulse',
            event_info=event_info,
            channel=channel,
        )

        self.event_feedback[id] = None

    def send_charge(self, id, execution_condition, time, channel, target_voltage):
        topic, service_type = self.ROS_SERVICE_REQUEST_EVENTS

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = execution_condition
        event_info.execution_time = float(time) if time is not None else 0.0

        charge = Charge(
            event_info=event_info,
            channel=channel,
            target_voltage=target_voltage,
        )
        request.charges.append(charge)

        response = self.call_service(client, request)
        assert response.success, "Failed charge request."

        self.printer.print_event(
            event_type='Charge',
            event_info=event_info,
            channel=channel,
        )

        self.event_feedback[id] = None

    def send_discharge(self, id, execution_condition, time, channel, target_voltage):
        topic, service_type = self.ROS_SERVICE_REQUEST_EVENTS

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = execution_condition
        event_info.execution_time = float(time) if time is not None else 0.0

        discharge = Discharge(
            event_info=event_info,
            channel=channel,
            target_voltage=target_voltage,
        )
        request.discharges.append(discharge)

        response = self.call_service(client, request)
        assert response.success, "Failed discharge request."

        self.printer.print_event(
            event_type='Discharge',
            event_info=event_info,
            channel=channel,
        )

        self.event_feedback[id] = None

    def send_trigger_out(self, id, execution_condition, time, port, duration_us):
        topic, service_type = self.ROS_SERVICE_REQUEST_EVENTS

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = execution_condition
        event_info.execution_time = float(time) if time is not None else 0.0

        trigger_out = TriggerOut(
            event_info=event_info,
            port=port,
            duration_us=duration_us,
        )
        request.trigger_outs.append(trigger_out)

        response = self.call_service(client, request)
        assert response.success, "Failed trigger out request."

        self.printer.print_event(
            event_type='Trigger out',
            event_info=event_info,
            port=port,
        )

        self.event_feedback[id] = None

    # Feedback

    def update_event_feedback(self, feedback):
        id = feedback.id
        error = feedback.error

        self.event_feedback[id] = error

    def get_event_feedback(self, id):
        if id not in self.event_feedback:
            return None

        return self.event_feedback[id]

    def handle_pulse_feedback(self, feedback):
        if self.verbose:
            self.printer.print_feedback('Pulse', feedback)

        self.update_event_feedback(feedback)

    def handle_charge_feedback(self, feedback):
        if self.verbose:
            self.printer.print_feedback('Charge', feedback)

        self.update_event_feedback(feedback)

    def handle_discharge_feedback(self, feedback):
        if self.verbose:
            self.printer.print_feedback('Discharge', feedback)

        self.update_event_feedback(feedback)

    def handle_trigger_out_feedback(self, feedback):
        if self.verbose:
            self.printer.print_feedback('Trigger out', feedback)

        self.update_event_feedback(feedback)

    # Targeting

    def get_target_voltages(self, target):
        topic, service_type = self.ROS_SERVICE_GET_TARGET_VOLTAGES

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        request.target = target

        value = self.call_service(client, request)
        assert value.success, "Invalid displacement, rotation angle, intensity, or algorithm."

        return value.voltages, value.reversed_polarities

    def get_maximum_intensity(self, displacement_x, displacement_y, rotation_angle, algorithm):
        topic, service_type = self.ROS_SERVICE_GET_MAXIMUM_INTENSITY

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        request.displacement_x = displacement_x
        request.displacement_y = displacement_y
        request.rotation_angle = rotation_angle
        request.algorithm = algorithm

        value = self.call_service(client, request)
        assert value.success, "Invalid displacement, rotation angle, or algorithm."

        return value.maximum_intensity

    def get_default_waveform(self, channel):
        topic, service_type = self.ROS_SERVICE_GET_DEFAULT_WAVEFORM

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        request.channel = channel

        value = self.call_service(client, request)
        assert value.success, "Invalid channel."

        return value.waveform

    def get_multipulse_waveforms(self, targets, target_waveforms):
        topic, service_type = self.ROS_SERVICE_GET_MULTIPULSE_WAVEFORMS

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        request.targets = targets
        request.target_waveforms = target_waveforms

        value = self.call_service(client, request)
        assert value.success, "Invalid number of pulses, targets, or target waveforms."

        return value.initial_voltages, value.approximated_waveforms

    def reverse_polarity(self, waveform):
        topic, service_type = self.ROS_SERVICE_REVERSE_POLARITY

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        request.waveform = waveform

        value = self.call_service(client, request)
        assert value.success, "Failed request."

        return value.waveform

    # Stimulation

    def is_stimulation_allowed(self):
        topic, service_type = self.ROS_SERVICE_IS_STIMULATION_ALLOWED

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        value = self.call_service(client, request)
        assert value.success, "Failed request."

        return value.stimulation_allowed

    # MEP analysis

    def analyze_mep(self, time, mep_configuration):
        topic, action_type = self.ROS_ACTION_ANALYZE_MEP

        client = self.ros_action_clients[topic]

        # Define goal.
        goal = action_type.Goal()

        goal.time = time
        goal.mep_configuration = mep_configuration

        # Send goal to ROS action.
        send_goal_future = client.send_goal_async(goal)
        rclpy.spin_until_future_complete(self, send_goal_future)

        goal_handle = send_goal_future.result()
        if not goal_handle.accepted:
            self.logger.info('Analyze MEP goal rejected.')
            return None, None

        # Get result from ROS action.
        get_result_future = goal_handle.get_result_async()

        # XXX: Because of the custom SIGINT handler (defined in the constructor of MTMSApi class),
        #   waiting for the MEP here cannot be interrupted by Ctrl+C. It turned out to be hard to
        #   keep ROS in a working state after Ctrl+C without overriding the default SIGINT handler,
        #   hence the current solution. Maybe this could be looked further into at some point.
        #
        rclpy.spin_until_future_complete(self, get_result_future)

        result = get_result_future.result()
        if result is None:
            self.logger.info('Analyze MEP result failed.')
            return None, None

        return result.result.mep, result.result.errors

    # Experiment
    def get_experiment_handler(self):
        return self.experiment_handler

    # System state

    def handle_system_state(self, system_state):
        self.system_state = system_state

    def wait_for_new_state(self):
        self.system_state = None
        while self.system_state is None:
            rclpy.spin_once(self)

    def print_state(self, force=False):
        self.printer.print_state(
            state=self.system_state,
            session=self.session,
            force=force,
        )

    # Decider

    def handle_decider_enabled(self, msg):
        enabled = msg.data
        if enabled:
            # Disable API completely if Decider is enabled; this is to prevent Decider from inadvertently charging
            # the channels to the maximum voltage (due to using PWM) while API is being used to deliver pulses.
            self.logger.error("Decider is enabled. All API operations are now disabled. Please restart.")
            self.destroy_node()
            rclpy.shutdown()

    # Session

    def handle_session(self, session):
        self.session = session
