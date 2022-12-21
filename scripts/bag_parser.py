import argparse
import sqlite3
import sys


from rosidl_runtime_py.utilities import get_message
from rclpy.serialization import deserialize_message


def parse_arguments():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("-b", "--bag-file", type=str, help="Full path to the bag file directory")
    arg_parser.add_argument("-t", "--topic", action="append", type=str, help="Topic to get from bag file")
    arg_parser.add_argument("-n", "--name", type=str, required=False, help="String that is prepended to output files")

    arg_parser.add_argument('--full', action='store_true',
                            help="If set, store full data (eeg channels, event types etc)?")
    arg_parser.set_defaults(feature=False)

    args = arg_parser.parse_args()
    if args.bag_file is None:
        print("ERROR, you must provide a bag file. Run script with --help")
        sys.exit(1)

    print(f"Using bag file {args.bag_file}")
    print(f"Looking for topics {', '.join(args.topic)}")
    if args.name is None:
        args.name = ""
    return args.bag_file, args.topic, args.name


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

    def get_messages(self, topic):
        if topic not in self.topic_id:
            print(f"Topic {topic} not found")
            return []

        topic_id = self.topic_id[topic]

        rows = self.cursor.execute(
            "SELECT data FROM messages WHERE topic_id = {}".format(topic_id)).fetchall()

        messages = [deserialize_message(data, self.topic_msg_message[topic]) for data, in rows]
        print(f"Found {len(messages)} messages for topic {topic}")
        return messages

    def save_topic_timestamps(self, topic, file_name):
        messages = self.get_messages(topic)
        timestamps = list(map(lambda sample: self.parse_sample_time(sample), messages))

        with open(file_name, 'w') as f:
            f.write("\n".join(timestamps))

    def save_topic_full(self, topic, file_name):
        messages = self.get_messages(topic)

        for message in messages:
            parsed = self.parse_sample_fields(message)
            print(parsed)

    def save_eeg(self, file_name):
        topic = '/eeg/raw_data'
        messages = self.get_messages(topic)

        timestamps = list(map(lambda sample: self.parse_sample_time(sample), messages))

        eeg_channels = list(map(lambda sample: self.parse_sample_eeg(sample), messages))

        write_data = []
        for i in range(len(timestamps)):
            s = ",".join([timestamps[i], *eeg_channels[i]])
            write_data.append(s)
        with open(file_name, 'w') as f:
            f.write("\n".join(write_data))

    def parse_sample_fields(self, sample):
        fields = sample.get_fields_and_field_types()

        field_values = []

        for field_name, field_type in fields.items():
            if 'sequence' in field_type:
                seq = self.parse_sequence(sample.field_name)
                field_value = ",".join(seq)
            else:
                field_value = str(sample.field_name)
            field_values.append(field_value)

        return field_values

    @staticmethod
    def parse_sequence(sequence):
        return [str(v) for v in sequence]

    @staticmethod
    def parse_sample_time(sample):
        return str(sample.time)

    @staticmethod
    def parse_sample_eeg(sample):
        eeg_channels = sample.eeg_channels
        cast_as_str = [str(channel) for channel in eeg_channels]
        return cast_as_str


bag_file, topics, fname = parse_arguments()
parser = BagFileParser(bag_file)


# Save topics.
for topic in topics:
    n = f"{fname}_" if len(fname) > 0 else ""
    output_file_name = f"{n}{topic[1:].replace('/', '_')}"
    parser.save_topic_timestamps(topic, output_file_name)

print("Done")
