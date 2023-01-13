import rclpy
from rclpy.action import ActionClient
from rclpy.node import Node

from fpga_interfaces.srv import StartDevice, StopDevice, StartExperiment, StopExperiment, SendEventTrigger, SendPulse, SendCharge, SendDischarge, SendSignalOut
from fpga_interfaces.msg import ChargeFeedback, DischargeFeedback, SignalOutFeedback, SystemState, \
    WaveformPiece, PulseFeedback, Event, ChargeError, PulseError, DischargeError, SignalOutError
from mtms_interfaces.action import AnalyzeMep
from mtms_interfaces.srv import GetChannelVoltages
from targeting_interfaces.srv import GetDefaultWaveform, ReversePolarity

from MTMSApiPrinter import MTMSApiPrinter

class MTMSApiNode(Node):
    # To FPGA
    ROS_SERVICE_START_DEVICE = ('/fpga/start_device', StartDevice)
    ROS_SERVICE_STOP_DEVICE = ('/fpga/stop_device', StopDevice)
    ROS_SERVICE_START_EXPERIMENT = ('/fpga/start_experiment', StartExperiment)
    ROS_SERVICE_STOP_EXPERIMENT = ('/fpga/stop_experiment', StopExperiment)
    ROS_SERVICE_SEND_EVENT_TRIGGER = ('/fpga/send_event_trigger', SendEventTrigger)
    ROS_SERVICE_SEND_PULSE = ('/fpga/send_pulse', SendPulse)
    ROS_SERVICE_SEND_CHARGE = ('/fpga/send_charge', SendCharge)
    ROS_SERVICE_SEND_DISCHARGE = ('/fpga/send_discharge', SendDischarge)
    ROS_SERVICE_SEND_SIGNAL_OUT = ('/fpga/send_signal_out', SendSignalOut)

    # To other parts of the system
    ROS_SERVICE_GET_CHANNEL_VOLTAGES = ('/targeting/get_channel_voltages', GetChannelVoltages)
    ROS_SERVICE_GET_DEFAULT_WAVEFORM = ('/waveforms/get_default', GetDefaultWaveform)
    ROS_SERVICE_REVERSE_POLARITY = ('/waveforms/reverse_polarity', ReversePolarity)

    ROS_ACTION_ANALYZE_MEP = ('/emg/analyze_mep', AnalyzeMep)

    ROS_SERVICES = (
        ROS_SERVICE_START_DEVICE,
        ROS_SERVICE_STOP_DEVICE,
        ROS_SERVICE_START_EXPERIMENT,
        ROS_SERVICE_STOP_EXPERIMENT,
        ROS_SERVICE_SEND_EVENT_TRIGGER,
        ROS_SERVICE_SEND_PULSE,
        ROS_SERVICE_SEND_CHARGE,
        ROS_SERVICE_SEND_DISCHARGE,
        ROS_SERVICE_SEND_SIGNAL_OUT,
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

        self.ros_service_clients = {}

        for topic, service_object in self.ROS_SERVICES:
            client = self.create_client(service_object, topic)
            while not client.wait_for_service(timeout_sec=1.0):
                self.get_logger().info('Service {} not available, waiting...'.format(topic))

            self.ros_service_clients[topic] = client

        self.ros_action_clients = {}

        for topic, action_object in self.ROS_ACTIONS:
            client = ActionClient(self, action_object, topic)
            while not client.wait_for_server(timeout_sec=1.0):
                self.get_logger().info('Action {} not available, waiting...'.format(topic))

            self.ros_action_clients[topic] = client

        # Have a queue of only one message so that only the latest system state is ever received.
        #
        self.system_state_subscriber = self.create_subscription(SystemState, '/fpga/system_state', self.handle_system_state, 1)

        self.pulse_feedback_subscriber = self.create_subscription(PulseFeedback, '/fpga/pulse_feedback', self.handle_pulse_feedback, 10)
        self.charge_feedback_subscriber = self.create_subscription(ChargeFeedback, '/fpga/charge_feedback', self.handle_charge_feedback, 10)
        self.discharge_feedback_subscriber = self.create_subscription(DischargeFeedback, '/fpga/discharge_feedback', self.handle_discharge_feedback, 10)
        self.signal_out_feedback_subscriber = self.create_subscription(SignalOutFeedback, '/fpga/signal_out_feedback', self.handle_signal_out_feedback, 10)

        self.event_feedback = {}

    def call_service(self, client, request):
        self.future = client.call_async(request)
        rclpy.spin_until_future_complete(self, self.future)
        return self.future.result()

    # Starting and stopping

    def start_device(self):
        topic, service_object = self.ROS_SERVICE_START_DEVICE

        client = self.ros_service_clients[topic]
        request = service_object.Request()

        return self.call_service(client, request)

    def stop_device(self):
        topic, service_object = self.ROS_SERVICE_STOP_DEVICE

        client = self.ros_service_clients[topic]
        request = service_object.Request()

        return self.call_service(client, request)

    def start_experiment(self):
        topic, service_object = self.ROS_SERVICE_START_EXPERIMENT

        client = self.ros_service_clients[topic]
        request = service_object.Request()

        return self.call_service(client, request)

    def stop_experiment(self):
        topic, service_object = self.ROS_SERVICE_STOP_EXPERIMENT

        client = self.ros_service_clients[topic]
        request = service_object.Request()

        return self.call_service(client, request)

    # Events

    def send_event_trigger(self):
        topic, service_object = self.ROS_SERVICE_SEND_EVENT_TRIGGER

        client = self.ros_service_clients[topic]
        request = service_object.Request()

        value = self.call_service(client, request)

        self.printer.print_event_trigger()

        return value

    def send_pulse(self, id, execution_condition, time, channel, waveform):
        topic, service_object = self.ROS_SERVICE_SEND_PULSE

        client = self.ros_service_clients[topic]
        request = service_object.Request()

        event = Event()
        event.id = id
        event.execution_condition.value = execution_condition
        event.time = float(time)

        request.pulse.event = event
        request.pulse.channel = channel
        request.pulse.waveform = waveform

        value = self.call_service(client, request)

        self.printer.print_event(
            event_type='Pulse',
            event=event,
            channel=channel,
        )

        self.event_feedback[id] = None

        return value

    def send_charge(self, id, execution_condition, time, channel, target_voltage):
        topic, service_object = self.ROS_SERVICE_SEND_CHARGE

        client = self.ros_service_clients[topic]
        request = service_object.Request()

        event = Event()
        event.id = id
        event.execution_condition.value = execution_condition
        event.time = float(time)

        request.charge.event = event
        request.charge.channel = channel
        request.charge.target_voltage = target_voltage

        value = self.call_service(client, request)

        self.printer.print_event(
            event_type='Charge',
            event=event,
            channel=channel,
        )

        self.event_feedback[id] = None

        return value

    def send_discharge(self, id, execution_condition, time, channel, target_voltage):
        topic, service_object = self.ROS_SERVICE_SEND_DISCHARGE

        client = self.ros_service_clients[topic]
        request = service_object.Request()

        event = Event()
        event.id = id
        event.execution_condition.value = execution_condition
        event.time = float(time)

        request.discharge.event = event
        request.discharge.channel = channel
        request.discharge.target_voltage = target_voltage

        value = self.call_service(client, request)

        self.printer.print_event(
            event_type='Discharge',
            event=event,
            channel=channel,
        )

        self.event_feedback[id] = None

        return value

    def send_signal_out(self, id, execution_condition, time, port, duration_us):
        topic, service_object = self.ROS_SERVICE_SEND_SIGNAL_OUT

        client = self.ros_service_clients[topic]
        request = service_object.Request()

        event = Event()
        event.id = id
        event.execution_condition.value = execution_condition
        event.time = float(time)

        request.signal_out.event = event
        request.signal_out.port = port
        request.signal_out.duration_us = duration_us

        value = self.call_service(client, request)

        self.printer.print_event(
            event_type='Signal out',
            event=event,
            port=port,
        )

        self.event_feedback[id] = None

        return value

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
        topic, service_object = self.ROS_SERVICE_GET_CHANNEL_VOLTAGES

        client = self.ros_service_clients[topic]
        request = service_object.Request()

        request.displacement_x = displacement_x
        request.displacement_y = displacement_y
        request.rotation_angle = rotation_angle
        request.intensity = intensity

        value = self.call_service(client, request)
        assert value.success, "Invalid displacement, rotation angle, or intensity."

        return value.voltages, value.reversed_polarities

    def get_default_waveform(self, channel):
        topic, service_object = self.ROS_SERVICE_GET_DEFAULT_WAVEFORM

        client = self.ros_service_clients[topic]
        request = service_object.Request()

        request.channel = channel

        value = self.call_service(client, request)
        assert value.success, "Invalid channel."

        return value.waveform

    def reverse_polarity(self, waveform):
        topic, service_object = self.ROS_SERVICE_REVERSE_POLARITY

        client = self.ros_service_clients[topic]
        request = service_object.Request()

        request.waveform = waveform

        value = self.call_service(client, request)
        assert value.success, "Failed request."

        return value.waveform

    # MEP analysis

    def analyze_mep(self, time, emg_channel):
        topic, action_object = self.ROS_ACTION_ANALYZE_MEP

        client = self.ros_action_clients[topic]

        # Define goal.
        goal = action_object.Goal()

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
