import argparse
import sqlite3
import sys
import gc

from rosidl_runtime_py.utilities import get_message
from rclpy.serialization import deserialize_message


def parse_arguments():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("-b", "--bag-file", type=str, help="Full path to the bag file directory")
    arg_parser.add_argument("-t", "--topic", action="append", type=str, help="Topic to get from bag file")
    arg_parser.add_argument("-p", "--prefix", type=str, required=False, help="String that is prefixed to output files")

    arg_parser.add_argument('--full', action='store_true',
                            help="If set, store full data (eeg channels, event types etc)")
    arg_parser.set_defaults(feature=False)

    args = arg_parser.parse_args()
    if args.bag_file is None:
        print("ERROR, you must provide a bag file. Run script with --help")
        sys.exit(1)

    print(f"Using bag file {args.bag_file}")
    print(f"Looking for topics {', '.join(args.topic)}")
    if args.prefix is None:
        args.prefix = ""
    return args.bag_file, args.topic, args.prefix, args.full


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
            f"SELECT data FROM messages WHERE topic_id = {topic_id}").fetchall()

        print(f"Found {len(rows)} messages for topic {topic}")

        # Utilize yield so we do not need to fit all data into memory (apart from rows)
        for data, in rows:
            yield deserialize_message(data, self.topic_msg_message[topic])

    def save_topic_timestamps(self, topic, file_name):
        messages = self.get_messages(topic)
        timestamps = list(map(lambda sample: str(sample.time), messages))

        with open(file_name, 'w') as f:
            f.write("\n".join(timestamps))

    def save_topic_full(self, topic, file_name):
        f = open(file_name, 'w')
        for message in self.get_messages(topic):
            parsed = self.parse_message_fields(message)
            f.write(parsed + "\n")
        f.close()

    def parse_message_fields(self, message):
        fields = message.get_fields_and_field_types()

        field_values = []

        for field_name, field_type in fields.items():
            if 'sequence' in field_type:
                seq = getattr(message, field_name)
                field_value = ",".join(str(element) for element in seq)
            else:
                field_value = str(getattr(message, field_name))

            # Time should be the first column.
            if field_name == "time":
                field_values.insert(0, field_value)
            else:
                field_values.append(field_value)

        return ",".join(field_values)


bag_file, topics, prefix, full = parse_arguments()
parser = BagFileParser(bag_file)

# Save topics.
for topic in topics:
    prefix_formatted = f"{prefix}_" if len(prefix) > 0 else ""

    # Add prefix, drop the leading / and replace remaining / with _.
    # For instance, with prefix "experiment1", and topic "/eeg/raw_data", yields "experiment1_eeg_raw_data"
    output_file_name = f"{prefix_formatted}{topic[1:].replace('/', '_')}"

    if full:
        parser.save_topic_full(topic, output_file_name)
    else:
        parser.save_topic_timestamps(topic, output_file_name)

print("Done")
