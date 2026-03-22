import os
import re
from datetime import datetime

from mtms_experiment_interfaces.srv import LogTrial

from std_msgs.msg import Empty

import rclpy
from rclpy.node import Node


class TrialLoggerNode(Node):

    HEARTBEAT_TOPIC = '/mtms/trial/log/heartbeat'
    HEARTBEAT_PUBLISH_PERIOD_S = 0.5

    TRIAL_COLUMNS = [
        "Trial index",
        "Number of attempts",
        "Trial start time (s)",
        "Number of targets",
        "x (mm)",
        "y (mm)",
        "Angle (deg)",
        "Intensity (V/m)",
        "x (mm, second target)",
        "y (mm, second target)",
        "Angle (deg, second target)",
        "Intensity (V/m, second target)",
        "MEP amplitude (uV)",
        "MEP latency (s)",
    ]

    def __init__(self):
        super().__init__('trial_logger_node')

        self.logger = self.get_logger()

        self.heartbeat_publisher = self.create_publisher(Empty, self.HEARTBEAT_TOPIC, 10)
        self.create_timer(self.HEARTBEAT_PUBLISH_PERIOD_S, lambda: self.heartbeat_publisher.publish(Empty()))

        self.logs_dir = os.getenv('MTMS_EXPERIMENT_LOGS_DIR')
        if not self.logs_dir:
            raise RuntimeError('MTMS_EXPERIMENT_LOGS_DIR is not set.')

        self.current_experiment_id = None
        self.current_log_file = None
        self.current_log_filename = None

        # Create service for logging trial.

        self.service = self.create_service(
            LogTrial,
            '/mtms/trial/log',
            self.log_trial_callback,
        )

        self.get_logger().info('Trial logs directory: {}.'.format(self.logs_dir))

    def open_new_log_file(self, experiment_name, subject_name, experiment_id):
        subject_name = self.sanitize_filename(subject_name)
        experiment_name = self.sanitize_filename(experiment_name)

        basepath = self.logs_dir

        # Ensure the directory exists
        if not os.path.exists(basepath):
            os.makedirs(basepath)

        current_timestamp = datetime.now().strftime("%Y-%m-%d-%H-%M-%S")
        filename_parts = [current_timestamp]
        if experiment_name:
            filename_parts.append(experiment_name)
        if subject_name:
            filename_parts.append(subject_name)
        filename = "_".join(filename_parts) + ".csv"
        filepath = os.path.join(basepath, filename)

        file = open(filepath, 'w+')
        self.current_log_filename = filename
        self.get_logger().info('New log file created!')
        self.get_logger().info('Logging into the file: {}.'.format(filename))

        return file

    def sanitize_filename(self, filename):
        # Replace any characters not in the whitelist with an underscore. The whitelist consists
        # of alphanumeric characters, period, and hyphen.
        return re.sub(r"[^a-zA-Z0-9\.\-]", "_", filename)

    def write_header(self, file):
        header = ";".join(self.TRIAL_COLUMNS) + '\n'
        file.write(header)

    def log_trial_row(self, file, trial_number, trial, num_of_attempts, mep_amplitude, mep_latency):
        num_of_targets = len(trial.targets)
        assert num_of_targets <= 2, "Does not support more than two targets."

        first_target = trial.targets[0]
        if num_of_targets == 2:
            second_target = trial.targets[1]
        else:
            second_target = None

        row = "{};{};{:.3f};{};{};{};{};{};{};{};{};{};{:.1f};{:.4f}\n".format(
            trial_number,
            num_of_attempts,
            trial.start_time,
            num_of_targets,
            first_target.displacement_x,
            first_target.displacement_y,
            first_target.rotation_angle,
            first_target.intensity,
            second_target.displacement_x if second_target else '',
            second_target.displacement_y if second_target else '',
            second_target.rotation_angle if second_target else '',
            second_target.intensity if second_target else '',
            mep_amplitude,
            mep_latency,
        )
        file.write(row)

    def rotate_log_file_if_needed(self, experiment_name, subject_name, experiment_id):
        if self.current_experiment_id == experiment_id and self.current_log_file is not None:
            return

        if self.current_log_file is not None:
            try:
                self.current_log_file.close()
            except Exception:
                pass

        self.current_experiment_id = experiment_id
        self.current_log_file = self.open_new_log_file(experiment_name, subject_name, experiment_id)
        self.write_header(self.current_log_file)

    def log_trial(self, experiment_name, subject_name, experiment_id, trial_number, trial, num_of_attempts, mep_amplitude, mep_latency):
        self.rotate_log_file_if_needed(experiment_name, subject_name, experiment_id)
        file = self.current_log_file

        self.log_trial_row(
            file=file,
            trial_number=trial_number,
            trial=trial,
            num_of_attempts=num_of_attempts,
            mep_amplitude=mep_amplitude,
            mep_latency=mep_latency,
        )
        file.flush()

    def log_trial_callback(self, request, response):
        experiment_name = request.experiment_name
        subject_name = request.subject_name
        experiment_id = request.experiment_id
        trial_number = request.trial_number
        trial = request.trial
        num_of_attempts = request.num_of_attempts
        mep_amplitude = request.mep_amplitude
        mep_latency = request.mep_latency

        self.get_logger().info('Logging trial {} (experiment_id={}).'.format(trial_number, experiment_id))

        self.log_trial(
            experiment_name=experiment_name,
            subject_name=subject_name,
            experiment_id=experiment_id,
            trial_number=trial_number,
            trial=trial,
            num_of_attempts=num_of_attempts,
            mep_amplitude=mep_amplitude,
            mep_latency=mep_latency,
        )
        response.success = True

        self.get_logger().info('Done.')

        return response

    def destroy_node(self):
        if self.current_log_file is not None:
            try:
                self.current_log_file.close()
            except Exception:
                pass
            self.current_log_file = None
        super().destroy_node()

def main(args=None):
    rclpy.init(args=args)

    trial_logger_node = TrialLoggerNode()

    rclpy.spin(trial_logger_node)

    rclpy.shutdown()


if __name__ == '__main__':
    main()
