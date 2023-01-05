from enum import Enum

import numpy as np

# TODO: Rename to be more generic, e.g., EegEmgDatapoint.
#
from mtms_interfaces.msg import EegDatapoint

from rclpy.callback_groups import MutuallyExclusiveCallbackGroup


class DataGatheringState(Enum):
    CHECK_IF_VALID_REQUEST = 0
    WAIT_FOR_MEP = 1
    GATHER_DATA = 2
    FINAL_STATE__FAILURE = 3
    FINAL_STATE__SUCCESS = 4


class DataGatherer:
    INITIAL_STATE = DataGatheringState.CHECK_IF_VALID_REQUEST
    FINAL_STATES = (
        DataGatheringState.FINAL_STATE__FAILURE,
        DataGatheringState.FINAL_STATE__SUCCESS,
    )

    # TODO: Replace with a proper duration once the MEP analysis algorithm is in place.
    #
    MEP_DURATION_S = 0.1

    # When determining if samples have been dropped by comparing the timestamps of two consecutive
    # samples, allow some tolerance to account for finite precision of floating point numbers.
    #
    TOLERANCE_S = 10 ** -6

    # Use relatively long queue to prevent dropping samples.
    #
    EMG_DATA_SUBSCRIBER_QUEUE_SIZE = 1024

    def __init__(self, goal_id, emg_channel, start_time, create_subscription, destroy_subscription, logger, sampling_frequency):
        self.goal_id = goal_id
        self.emg_channel = emg_channel
        self.start_time = start_time
        self.create_subscription = create_subscription
        self.destroy_subscription = destroy_subscription
        self.logger = logger
        self.sampling_frequency = sampling_frequency

        self.state = DataGatheringState(self.INITIAL_STATE)

        self.buffer_size = int(np.ceil(self.MEP_DURATION_S * self.sampling_frequency))
        self.emg_buffer = np.empty(self.buffer_size)
        self.time_buffer = np.empty(self.buffer_size)
        self.n_samples = 0

        self.sampling_period = 1 / self.sampling_frequency

        self.previous_time = None

        self.logger.info('{}: Waiting for EMG samples...'.format(self.goal_id))

        self.data_subscriber = self.create_subscription(
            EegDatapoint,
            '/eeg/raw_data',
            self.emg_data_callback,
            self.EMG_DATA_SUBSCRIBER_QUEUE_SIZE,

            # Note: Without explicitly defining callback group to be mutually exclusive, it occasionally
            # happens that - after having served a few actions - the ROS node ends up in a state in
            # which the EMG callback is never called for new actions, even though messages are published
            # in the topic.
            #
            callback_group=MutuallyExclusiveCallbackGroup(),
        )

    def check_dropped_samples(self, current_time):
        if self.previous_time is not None:
            time_diff = current_time - self.previous_time
            threshold = self.sampling_period + self.TOLERANCE_S

            if time_diff > threshold:
                self.logger.warn('{}: Sample(s) dropped. Time difference between consecutive samples: {:.4f}, should be: {:.4f}'.format(
                    self.goal_id,
                    time_diff,
                    threshold,
                ))

        self.previous_time = current_time

    def handle_state__check_if_valid_request(self, current_time, msg):
        channel_count = len(msg.eeg_channels)

        if current_time >= self.start_time:
            self.logger.warn('{}: Failure: Current time ({:.2f} s) is past the starting time ({:.2f} s).'.format(
                self.goal_id,
                current_time,
                self.start_time,
            ))
            self.state = DataGatheringState.FINAL_STATE__FAILURE

        elif self.emg_channel >= channel_count:
            self.logger.warn('{}: Failure: Requested channel ({}) larger than channel count ({}).'.format(
                self.goal_id,
                self.emg_channel,
                channel_count,
            ))
            self.state = DataGatheringState.FINAL_STATE__FAILURE

        else:
            self.logger.info('{}: Waiting for the starting time...'.format(self.goal_id))
            self.state = DataGatheringState.WAIT_FOR_MEP

    def handle_state__wait_for_mep(self, current_time):
        if current_time >= self.start_time:
            self.logger.info('{}: Gathering data...'.format(self.goal_id))
            self.state = DataGatheringState.GATHER_DATA

    def handle_state__gather_data(self, current_time, msg):
        if current_time < self.start_time + self.MEP_DURATION_S:
            self.emg_buffer[self.n_samples] = msg.eeg_channels[self.emg_channel]
            self.time_buffer[self.n_samples] = current_time - self.start_time

            self.n_samples += 1

        else:
            self.logger.info('{}: Finished gathering data.'.format(self.goal_id))
            self.state = DataGatheringState.FINAL_STATE__SUCCESS

    def emg_data_callback(self, msg):
        current_time = msg.time

        self.check_dropped_samples(current_time)

        if self.state == DataGatheringState.CHECK_IF_VALID_REQUEST:
            self.handle_state__check_if_valid_request(current_time, msg)

        elif self.state == DataGatheringState.WAIT_FOR_MEP:
            self.handle_state__wait_for_mep(current_time)

        elif self.state == DataGatheringState.GATHER_DATA:
            self.handle_state__gather_data(current_time, msg)

        elif self.state == DataGatheringState.FINAL_STATE__FAILURE:
            pass

        elif self.state == DataGatheringState.FINAL_STATE__SUCCESS:
            pass

        else:
            assert False, "Unknown state"

    def is_finished(self):
        return self.state in self.FINAL_STATES

    def success(self):
        return self.state == DataGatheringState.FINAL_STATE__SUCCESS

    def get_buffers(self):
        assert self.is_finished(), "Attempting to get unfinished buffers."

        return self.emg_buffer[:self.n_samples], self.time_buffer[:self.n_samples]

    def destroy(self):
        success = self.destroy_subscription(self.data_subscriber)
        assert success, "Could not unsubscribe from EMG data."
