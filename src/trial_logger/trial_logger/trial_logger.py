import os
import re
from datetime import datetime

from experiment_interfaces.srv import LogTrial

import rclpy
from rclpy.node import Node


class TrialLoggerNode(Node):

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

    def open_new_log_file(self, metadata, experiment_id):
        subject_name = self.sanitize_filename(metadata.subject_name)
        experiment_name = self.sanitize_filename(metadata.experiment_name)

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

    def log_trial_row(self, file, trial_number, trial, trial_result, num_of_attempts):
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
            trial_result.actual_start_time,
            num_of_targets,
            first_target.displacement_x,
            first_target.displacement_y,
            first_target.rotation_angle,
            first_target.intensity,
            second_target.displacement_x if second_target else '',
            second_target.displacement_y if second_target else '',
            second_target.rotation_angle if second_target else '',
            second_target.intensity if second_target else '',
            trial_result.mep_amplitude,
            trial_result.mep_latency,
        )
        file.write(row)

    def rotate_log_file_if_needed(self, metadata, experiment_id):
        if self.current_experiment_id == experiment_id and self.current_log_file is not None:
            return

        if self.current_log_file is not None:
            try:
                self.current_log_file.close()
            except Exception:
                pass

        self.current_experiment_id = experiment_id
        self.current_log_file = self.open_new_log_file(metadata, experiment_id)
        self.write_header(self.current_log_file)

    def log_trial(self, metadata, experiment_id, trial_number, trial, trial_result, num_of_attempts):
        self.rotate_log_file_if_needed(metadata, experiment_id)
        file = self.current_log_file

        self.log_trial_row(
            file=file,
            trial_number=trial_number,
            trial=trial,
            trial_result=trial_result,
            num_of_attempts=num_of_attempts,
        )
        file.flush()

    def log_trial_callback(self, request, response):
        metadata = request.metadata
        experiment_id = request.experiment_id
        trial_number = request.trial_number
        trial = request.trial
        trial_result = request.trial_result
        num_of_attempts = request.num_of_attempts

        self.get_logger().info('Logging trial {} (experiment_id={}).'.format(trial_number, experiment_id))

        self.log_trial(
            metadata=metadata,
            experiment_id=experiment_id,
            trial_number=trial_number,
            trial=trial,
            trial_result=trial_result,
            num_of_attempts=num_of_attempts,
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
