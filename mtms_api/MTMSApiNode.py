import rclpy
from rclpy.node import Node

from fpga_interfaces.srv import StartDevice, StopDevice, StartExperiment, StopExperiment, SendEventTrigger, SendPulse, SendCharge, SendDischarge, SendSignalOut
from fpga_interfaces.msg import SystemState, PulsePiece, Feedback, Event

from .MTMSApiPrinter import MTMSApiPrinter

from .enums.ChargeError import ChargeError
from .enums.DischargeError import DischargeError
from .enums.PulseError import PulseError
from .enums.SignalOutError import SignalOutError

class MTMSApiNode(Node):
    ROS_SERVICE_START_DEVICE = ('/fpga/start_device', StartDevice)
    ROS_SERVICE_STOP_DEVICE = ('/fpga/stop_device', StopDevice)
    ROS_SERVICE_START_EXPERIMENT = ('/fpga/start_experiment', StartExperiment)
    ROS_SERVICE_STOP_EXPERIMENT = ('/fpga/stop_experiment', StopExperiment)
    ROS_SERVICE_SEND_EVENT_TRIGGER = ('/fpga/send_event_trigger', SendEventTrigger)
    ROS_SERVICE_SEND_PULSE = ('/fpga/send_pulse', SendPulse)
    ROS_SERVICE_SEND_CHARGE = ('/fpga/send_charge', SendCharge)
    ROS_SERVICE_SEND_DISCHARGE = ('/fpga/send_discharge', SendDischarge)
    ROS_SERVICE_SEND_SIGNAL_OUT = ('/fpga/send_signal_out', SendSignalOut)

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
    )

    def __init__(self):
        super().__init__('mtms_api_node')

        self.printer = MTMSApiPrinter()

        self.system_state = None

        self.ros_services = {}

        for topic, service_object in self.ROS_SERVICES:
            client = self.create_client(service_object, topic)
            request = service_object.Request()
            while not client.wait_for_service(timeout_sec=1.0):
                self.get_logger().info('Service {} not available, waiting...'.format(topic))

            self.ros_services[topic] = {
                'client': client,
                'request': request,
            }

        # Have a queue of only one message so that only the latest system state is ever received.
        #
        self.system_state_subscriber = self.create_subscription(SystemState, '/fpga/system_state', self.handle_system_state, 1)

        self.pulse_feedback_subscriber = self.create_subscription(Feedback, '/fpga/pulse_feedback', self.handle_pulse_feedback, 10)
        self.charge_feedback_subscriber = self.create_subscription(Feedback, '/fpga/charge_feedback', self.handle_charge_feedback, 10)
        self.discharge_feedback_subscriber = self.create_subscription(Feedback, '/fpga/discharge_feedback', self.handle_discharge_feedback, 10)
        self.signal_out_feedback_subscriber = self.create_subscription(Feedback, '/fpga/signal_out_feedback', self.handle_signal_out_feedback, 10)

    def call_service(self, client, request):
        self.future = client.call_async(request)
        rclpy.spin_until_future_complete(self, self.future)
        return self.future.result()

    # Starting and stopping

    def start_device(self):
        topic, service_object = self.ROS_SERVICE_START_DEVICE

        ros_service = self.ros_services[topic]
        client = ros_service['client']
        request = ros_service['request']

        return self.call_service(client, request)

    def stop_device(self):
        topic, service_object = self.ROS_SERVICE_STOP_DEVICE

        ros_service = self.ros_services[topic]
        client = ros_service['client']
        request = ros_service['request']

        return self.call_service(client, request)

    def start_experiment(self):
        topic, service_object = self.ROS_SERVICE_START_EXPERIMENT

        ros_service = self.ros_services[topic]
        client = ros_service['client']
        request = ros_service['request']

        return self.call_service(client, request)

    def stop_experiment(self):
        topic, service_object = self.ROS_SERVICE_STOP_EXPERIMENT

        ros_service = self.ros_services[topic]
        client = ros_service['client']
        request = ros_service['request']

        return self.call_service(client, request)

    # Events

    def send_event_trigger(self):
        topic, service_object = self.ROS_SERVICE_SEND_EVENT_TRIGGER

        ros_service = self.ros_services[topic]
        client = ros_service['client']
        request = ros_service['request']

        return self.call_service(client, request)

    def send_pulse(self, id, execution_condition, time_us, channel, waveform):
        topic, service_object = self.ROS_SERVICE_SEND_PULSE

        ros_service = self.ros_services[topic]
        client = ros_service['client']
        request = ros_service['request']

        event = Event()
        event.id = id
        event.execution_condition = execution_condition.value
        event.time_us = time_us

        request.pulse.event = event
        request.pulse.channel = channel
        for piece in waveform:
            new_piece = PulsePiece()
            new_piece.mode = piece['mode'].value
            new_piece.duration_in_ticks = piece['duration_in_ticks']
            request.pulse.pieces.append(new_piece)

        value = self.call_service(client, request)

        self.printer.print_event(
            event_type='Pulse',
            event=event,
            channel=channel,
        )

        return value

    def send_charge(self, id, execution_condition, time_us, channel, target_voltage):
        topic, service_object = self.ROS_SERVICE_SEND_CHARGE

        ros_service = self.ros_services[topic]
        client = ros_service['client']
        request = ros_service['request']

        event = Event()
        event.id = id
        event.execution_condition = execution_condition.value
        event.time_us = time_us

        request.charge.event = event
        request.charge.channel = channel
        request.charge.target_voltage = target_voltage

        value = self.call_service(client, request)

        self.printer.print_event(
            event_type='Charge',
            event=event,
            channel=channel,
        )

        return value

    def send_discharge(self, id, execution_condition, time_us, channel, target_voltage):
        topic, service_object = self.ROS_SERVICE_SEND_DISCHARGE

        ros_service = self.ros_services[topic]
        client = ros_service['client']
        request = ros_service['request']

        event = Event()
        event.id = id
        event.execution_condition = execution_condition.value
        event.time_us = time_us

        request.discharge.event = event
        request.discharge.channel = channel
        request.discharge.target_voltage = target_voltage

        value = self.call_service(client, request)

        self.printer.print_event(
            event_type='Discharge',
            event=event,
            channel=channel,
        )

        return value

    def send_signal_out(self, id, execution_condition, time_us, port, duration_us):
        topic, service_object = self.ROS_SERVICE_SEND_SIGNAL_OUT

        ros_service = self.ros_services[topic]
        client = ros_service['client']
        request = ros_service['request']

        event = Event()
        event.id = id
        event.execution_condition = execution_condition.value
        event.time_us = time_us

        request.signal_out.event = event
        request.signal_out.port = port
        request.signal_out.duration_us = duration_us

        value = self.call_service(client, request)

        self.printer.print_event(
            event_type='Signal out',
            event=event,
            port=port,
        )

        return value

    # Feedback

    def handle_pulse_feedback(self, feedback):
        self.printer.print_feedback('Pulse', PulseError, feedback)

    def handle_charge_feedback(self, feedback):
        self.printer.print_feedback('Charge', ChargeError, feedback)

    def handle_discharge_feedback(self, feedback):
        self.printer.print_feedback('Discharge', DischargeError, feedback)

    def handle_signal_out_feedback(self, feedback):
        self.printer.print_feedback('Signal out', SignalOutError, feedback)

    # System state

    def handle_system_state(self, system_state):
        self.system_state = system_state

    def wait_for_new_state(self):
        self.system_state = None
        while self.system_state is None:
            rclpy.spin_once(self)

        self.printer.print_system_state(self.system_state)
