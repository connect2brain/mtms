import os
import re
from datetime import datetime

from experiment_interfaces.srv import LogTrial

import rclpy
from rclpy.node import Node


class TrialLoggerNode(Node):

    CSV_DIRECTORY = '/app/project/csv'

    TRIAL_COLUMNS = [
        "Trial index",
        "Number of attempts",
        "Trial finish time (s)",
        "x (mm)",
        "y (mm)",
        "Angle (deg)",
        "MEP amplitude (uV)",
        "MEP latency (s)",
    ]

    def __init__(self):
        super().__init__('trial_logger_node')

        self.logger = self.get_logger()

        # Create service for logging trial.

        self.service = self.create_service(
            LogTrial,
            '/trial/log',
            self.log_trial_callback,
        )

    def open_log_file(self, metadata):
        subject_name = self.sanitize_filename(metadata.subject_name)
        experiment_name = self.sanitize_filename(metadata.experiment_name)

        # Ensure the directory exists
        if not os.path.exists(self.CSV_DIRECTORY):
            os.makedirs(self.CSV_DIRECTORY)

        # The pattern below matches filenames with the format:
        #
        # YYYY-DD-MM-hh-mm-ss_[experiment_name]_[subject-name].csv
        pattern = re.compile(r"(\d{4}-\d{2}-\d{2}-\d{2}-\d{2}-\d{2})_" + re.escape(experiment_name) + "_" + re.escape(subject_name) + r"\.csv")

        matching_files = []
        for filename in os.listdir(self.CSV_DIRECTORY):
            if pattern.match(filename):
                matching_files.append(filename)

        # Sort the files based on their timestamp in descending order
        matching_files.sort(reverse=True)

        if matching_files:
            # Take the file with the latest timestamp
            filename = matching_files[0]
            filepath = os.path.join(self.CSV_DIRECTORY, filename)
            mode = 'a+'

            new_file_created = False
        else:
            # Create a new file
            current_timestamp = datetime.now().strftime("%Y-%m-%d-%H-%M-%S")
            filename = f"{current_timestamp}_{experiment_name}_{subject_name}.csv"
            filepath = os.path.join(self.CSV_DIRECTORY, filename)
            mode = 'w+'

            new_file_created = True
            self.get_logger().info('New log file created!')

        file = open(filepath, mode)
        self.get_logger().info('Logging into the file: {}.'.format(filename))

        return file, new_file_created

    def sanitize_filename(self, filename):
        # Replace any characters not in the whitelist with an underscore. The whitelist consists
        # of alphanumeric characters, period, and hyphen.
        return re.sub(r"[^a-zA-Z0-9\.\-]", "_", filename)

    def write_header(self, file):
        header = ";".join(self.TRIAL_COLUMNS) + '\n'
        file.write(header)

    def log_trial_row(self, file, trial_index, trial, trial_result):
        assert len(trial.stimuli) == 1, "Does not support multistimulus trials yet!"

        stimulus = trial.stimuli[0]
        target = stimulus.target

        row = "{};{};{:.3f};{};{};{};{:.1f};{:.4f}\n".format(
            trial_index,
            trial_result.num_of_attempts,
            trial_result.trial_finish_time,
            target.displacement_x,
            target.displacement_y,
            target.rotation_angle,
            trial_result.mep.amplitude,
            trial_result.mep.latency,
        )
        file.write(row)

    def log_trial(self, metadata, trial_index, trial, trial_result):
        file, new_file_created = self.open_log_file(metadata)

        # If the file was just created, write the header as the first line.
        if new_file_created:
            self.write_header(file)

        self.log_trial_row(
            file=file,
            trial_index=trial_index,
            trial=trial,
            trial_result=trial_result,
        )

    def log_trial_callback(self, request, response):
        metadata = request.metadata
        trial_index = request.trial_index
        trial = request.trial
        trial_result = request.trial_result

        self.get_logger().info('Logging a trial with the index {}.'.format(trial_index))

        self.log_trial(
            metadata=metadata,
            trial_index=trial_index,
            trial=trial,
            trial_result=trial_result,
        )
        response.success = True

        self.get_logger().info('Done.')

        return response

def main(args=None):
    rclpy.init(args=args)

    trial_logger_node = TrialLoggerNode()

    rclpy.spin(trial_logger_node)

    rclpy.shutdown()


if __name__ == '__main__':
    main()
