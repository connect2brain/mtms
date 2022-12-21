import argparse
import sqlite3
from rosidl_runtime_py.utilities import get_message
from rclpy.serialization import deserialize_message

import numpy as np
from timeit import default_timer as timer


def parse_bag_file():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("-b", "--bag-file", type=str, help="Full path to the bag file directory")
    args = arg_parser.parse_args()
    if args.bag_file is None:
        print("ERROR, you must provide a bag file. Run script with --help")
        print()
        return ""

    print(f"Using bag file {args.bag_file}")
    return args.bag_file


"""
Adapted from https://answers.ros.org/question/358686/how-to-read-a-bag-file-in-ros2/
"""


class BagFileParser:
    def __init__(self, bag_file):
        self.conn = sqlite3.connect(bag_file)
        self.cursor = self.conn.cursor()

        topics_data = self.cursor.execute("SELECT id, name, type FROM topics").fetchall()
        self.topic_type = {name_of: type_of for id_of, name_of, type_of in topics_data}
        self.topic_id = {name_of: id_of for id_of, name_of, type_of in topics_data}
        self.topic_msg_message = {name_of: get_message(type_of) for id_of, name_of, type_of in topics_data}

    def __del__(self):
        self.conn.close()

    def get_messages(self, topic_name):
        topic_id = self.topic_id[topic_name]

        rows = self.cursor.execute(
            "SELECT data FROM messages WHERE topic_id = {}".format(topic_id)).fetchall()

        return [deserialize_message(data, self.topic_msg_message[topic_name]) for data, in rows]

    def save_topic_timestamps(self, topic, file_name):
        if topic not in self.topic_id:
            print(f"Topic {topic} not found")
            return

        messages = self.get_topic_messages(topic)
        timestamps = list(map(lambda sample: self.parse_sample_time(sample), messages))

        with open(file_name, 'w') as f:
            f.write("\n".join(timestamps))

    def save_eeg(self, file_name):
        topic = '/eeg/raw_data'
        messages = self.get_topic_messages(topic)

        timestamps = list(map(lambda sample: self.parse_sample_time(sample), messages))

        eeg_channels = list(map(lambda sample: self.parse_sample_eeg(sample), messages))

        write_data = []
        for i in range(len(timestamps)):
            s = ",".join([timestamps[i], *eeg_channels[i]])
            write_data.append(s)
        with open(file_name, 'w') as f:
            f.write("\n".join(write_data))

    def get_topic_messages(self, topic):
        if topic not in self.topic_id:
            print(f"Topic {topic} not found")
            return

        messages = self.get_messages(topic)
        print(f"Found {len(messages)} messages for topic {topic}")
        return messages

    @staticmethod
    def parse_sample_time(sample):
        return str(sample.time)

    @staticmethod
    def parse_sample_eeg(sample):
        eeg_channels = sample.eeg_channels
        cast_as_str = [str(channel) for channel in eeg_channels]
        return cast_as_str


bag_file = parse_bag_file()
parser = BagFileParser(bag_file)

# Save EEG.
parser.save_eeg("file_name")

# Save events.
parser.save_topic_timestamps("/mtms/events", "events_matilda")

print("Done")
