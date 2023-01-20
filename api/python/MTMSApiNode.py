import rclpy
from rclpy.action import ActionClient
from rclpy.node import Node

from mtms_device_interfaces.msg import SystemState
from mtms_device_interfaces.srv import StartDevice, StopDevice, StartExperiment, StopExperiment

from event_interfaces.msg import EventTrigger, Pulse, Charge, Discharge, SignalOut, \
    ChargeFeedback, DischargeFeedback, SignalOutFeedback, \
    WaveformPiece, PulseFeedback, EventInfo, ChargeError, PulseError, DischargeError, SignalOutError

from eeg_interfaces.action import AnalyzeMep

from targeting_interfaces.srv import GetChannelVoltages, GetDefaultWaveform, ReversePolarity

from MTMSApiPrinter import MTMSApiPrinter

class MTMSApiNode(Node):
    # To mTMS device
    ROS_SERVICE_START_DEVICE = ('/mtms_device/start_device', StartDevice)
    ROS_SERVICE_STOP_DEVICE = ('/mtms_device/stop_device', StopDevice)
    ROS_SERVICE_START_EXPERIMENT = ('/mtms_device/start_experiment', StartExperiment)
    ROS_SERVICE_STOP_EXPERIMENT = ('/mtms_device/stop_experiment', StopExperiment)

    ROS_MESSAGE_SEND_EVENT_TRIGGER = ('/event/send/event_trigger', EventTrigger)
    ROS_MESSAGE_SEND_PULSE = ('/event/send/pulse', Pulse)
    ROS_MESSAGE_SEND_CHARGE = ('/event/send/charge', Charge)
    ROS_MESSAGE_SEND_DISCHARGE = ('/event/send/discharge', Discharge)
    ROS_MESSAGE_SEND_SIGNAL_OUT = ('/event/send/signal_out', SignalOut)

    # To other parts of the system
    ROS_SERVICE_GET_CHANNEL_VOLTAGES = ('/targeting/get_channel_voltages', GetChannelVoltages)
    ROS_SERVICE_GET_DEFAULT_WAVEFORM = ('/waveforms/get_default', GetDefaultWaveform)
    ROS_SERVICE_REVERSE_POLARITY = ('/waveforms/reverse_polarity', ReversePolarity)

    ROS_ACTION_ANALYZE_MEP = ('/emg/analyze_mep', AnalyzeMep)

    ROS_MESSAGES = (
        ROS_MESSAGE_SEND_EVENT_TRIGGER,
        ROS_MESSAGE_SEND_PULSE,
        ROS_MESSAGE_SEND_CHARGE,
        ROS_MESSAGE_SEND_DISCHARGE,
        ROS_MESSAGE_SEND_SIGNAL_OUT,
    )

    ROS_SERVICES = (
        ROS_SERVICE_START_DEVICE,
        ROS_SERVICE_STOP_DEVICE,
        ROS_SERVICE_START_EXPERIMENT,
        ROS_SERVICE_STOP_EXPERIMENT,
        ROS_SERVICE_GET_CHANNEL_VOLTAGES,
        ROS_SERVICE_GET_DEFAULT_WAVEFORM,
        ROS_SERVICE_REVERSE_POLARITY,
    )

    ROS_ACTIONS = (
        ROS_ACTION_ANALYZE_MEP,
    )

    def __init__(self):
        super().__init__('mtms_api_node')

        self.printer = MTMSApiPrinter()

        self.system_state = None

        # Message publishers

        self.ros_message_publishers = {}

        for topic, message_type in self.ROS_MESSAGES:
            publisher = self.create_publisher(message_type, topic, 10)
            self.ros_message_publishers[topic] = publisher

        # Service clients

        self.ros_service_clients = {}

        for topic, service_type in self.ROS_SERVICES:
            client = self.create_client(service_type, topic)
            while not client.wait_for_service(timeout_sec=1.0):
                self.get_logger().info('Service {} not available, waiting...'.format(topic))

            self.ros_service_clients[topic] = client

        # Action clients

        self.ros_action_clients = {}

        for topic, action_type in self.ROS_ACTIONS:
            client = ActionClient(self, action_type, topic)
            while not client.wait_for_server(timeout_sec=1.0):
                self.get_logger().info('Action {} not available, waiting...'.format(topic))

            self.ros_action_clients[topic] = client

        # Have a queue of only one message so that only the latest system state is ever received.
        #
        self.system_state_subscriber = self.create_subscription(SystemState, '/mtms_device/system_state', self.handle_system_state, 1)

        self.pulse_feedback_subscriber = self.create_subscription(PulseFeedback, '/event/pulse_feedback', self.handle_pulse_feedback, 10)
        self.charge_feedback_subscriber = self.create_subscription(ChargeFeedback, '/event/charge_feedback', self.handle_charge_feedback, 10)
        self.discharge_feedback_subscriber = self.create_subscription(DischargeFeedback, '/event/discharge_feedback', self.handle_discharge_feedback, 10)
        self.signal_out_feedback_subscriber = self.create_subscription(SignalOutFeedback, '/event/signal_out_feedback', self.handle_signal_out_feedback, 10)

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

    def start_experiment(self):
        topic, service_type = self.ROS_SERVICE_START_EXPERIMENT

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        return self.call_service(client, request)

    def stop_experiment(self):
        topic, service_type = self.ROS_SERVICE_STOP_EXPERIMENT

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        return self.call_service(client, request)

    # Events

    def send_event_trigger(self):
        topic, message_type = self.ROS_MESSAGE_SEND_EVENT_TRIGGER

        publisher = self.ros_message_publishers[topic]
        message = message_type()

        publisher.publish(message)

        self.printer.print_event_trigger()

    def send_pulse(self, id, execution_condition, time, channel, waveform):
        topic, message_type = self.ROS_MESSAGE_SEND_PULSE

        publisher = self.ros_message_publishers[topic]
        message = message_type()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = execution_condition
        event_info.execution_time = float(time)

        message.event_info = event_info
        message.channel = channel
        message.waveform = waveform

        publisher.publish(message)

        self.printer.print_event(
            event_type='Pulse',
            event_info=event_info,
            channel=channel,
        )

        self.event_feedback[id] = None

    def send_charge(self, id, execution_condition, time, channel, target_voltage):
        topic, message_type = self.ROS_MESSAGE_SEND_CHARGE

        publisher = self.ros_message_publishers[topic]
        message = message_type()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = execution_condition
        event_info.execution_time = float(time)

        message.event_info = event_info
        message.channel = channel
        message.target_voltage = target_voltage

        publisher.publish(message)

        self.printer.print_event(
            event_type='Charge',
            event_info=event_info,
            channel=channel,
        )

        self.event_feedback[id] = None

    def send_discharge(self, id, execution_condition, time, channel, target_voltage):
        topic, message_type = self.ROS_MESSAGE_SEND_DISCHARGE

        publisher = self.ros_message_publishers[topic]
        message = message_type()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = execution_condition
        event_info.execution_time = float(time)

        message.event_info = event_info
        message.channel = channel
        message.target_voltage = target_voltage

        publisher.publish(message)

        self.printer.print_event(
            event_type='Discharge',
            event_info=event_info,
            channel=channel,
        )

        self.event_feedback[id] = None

    def send_signal_out(self, id, execution_condition, time, port, duration_us):
        topic, message_type = self.ROS_MESSAGE_SEND_SIGNAL_OUT

        publisher = self.ros_message_publishers[topic]
        message = message_type()

        event_info = EventInfo()
        event_info.id = id
        event_info.execution_condition.value = execution_condition
        event_info.execution_time = float(time)

        message.event_info = event_info
        message.port = port
        message.duration_us = duration_us

        publisher.publish(message)

        self.printer.print_event(
            event_type='Signal out',
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
        self.printer.print_feedback('Pulse', feedback)
        self.update_event_feedback(feedback)

    def handle_charge_feedback(self, feedback):
        self.printer.print_feedback('Charge', feedback)
        self.update_event_feedback(feedback)

    def handle_discharge_feedback(self, feedback):
        self.printer.print_feedback('Discharge', feedback)
        self.update_event_feedback(feedback)

    def handle_signal_out_feedback(self, feedback):
        self.printer.print_feedback('Signal out', feedback)
        self.update_event_feedback(feedback)

    # Targeting

    def get_channel_voltages(self, displacement_x, displacement_y, rotation_angle, intensity):
        topic, service_type = self.ROS_SERVICE_GET_CHANNEL_VOLTAGES

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        request.displacement_x = displacement_x
        request.displacement_y = displacement_y
        request.rotation_angle = rotation_angle
        request.intensity = intensity

        value = self.call_service(client, request)
        assert value.success, "Invalid displacement, rotation angle, or intensity."

        return value.voltages, value.reversed_polarities

    def get_default_waveform(self, channel):
        topic, service_type = self.ROS_SERVICE_GET_DEFAULT_WAVEFORM

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        request.channel = channel

        value = self.call_service(client, request)
        assert value.success, "Invalid channel."

        return value.waveform

    def reverse_polarity(self, waveform):
        topic, service_type = self.ROS_SERVICE_REVERSE_POLARITY

        client = self.ros_service_clients[topic]
        request = service_type.Request()

        request.waveform = waveform

        value = self.call_service(client, request)
        assert value.success, "Failed request."

        return value.waveform

    # MEP analysis

    def analyze_mep(self, time, emg_channel):
        topic, action_type = self.ROS_ACTION_ANALYZE_MEP

        client = self.ros_action_clients[topic]

        # Define goal.
        goal = action_type.Goal()

        goal.time = time
        goal.emg_channel = emg_channel

        # Send goal to ROS action.
        send_goal_future = client.send_goal_async(goal)
        rclpy.spin_until_future_complete(self, send_goal_future)

        goal_handle = send_goal_future.result()
        if not goal_handle.accepted:
            self.get_logger().info('Analyze MEP goal rejected.')
            return None, None

        # Get result from ROS action.
        get_result_future = goal_handle.get_result_async()
        rclpy.spin_until_future_complete(self, get_result_future)

        result = get_result_future.result()
        if result is None:
            self.get_logger().info('Analyze MEP result failed.')
            return None, None

        return result.result.amplitude, result.result.latency

    # System state

    def handle_system_state(self, system_state):
        self.system_state = system_state

    def wait_for_new_state(self):
        self.system_state = None
        while self.system_state is None:
            rclpy.spin_once(self)

        self.printer.print_system_state(self.system_state)
