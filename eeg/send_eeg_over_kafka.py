import argparse
import json
import os
import sys
import time

import dotenv
from mne.io import concatenate_raws, read_raw_edf
import numpy as np
import pykafka
from pykafka import KafkaClient

dotenv.load_dotenv('.env', verbose=False)   # Load configuration from env vars and .env -file


def get_kafka_client(ip=None, port=None):
    """Initializes and returns a KafkaClient.

    Parameters
    ----------
    ip : str
        A valid IP address of the Kafka server.
    port : str or int
        The port of the Kafka server.

    Returns
    -------
    KafkaClient or None
        An initialized KafkaClient on success, or None in case of failure.
    """
    try:
        client = KafkaClient(hosts="{ip}:{port}".format(
            ip=(ip or os.getenv("KAFKA_IP") or '127.0.0.1'), 
            port=(port or os.getenv("KAFKA_PORT") or '9092')))
        return client
    except pykafka.exceptions.NoBrokersAvailableError as e:
        return None


def stream_data(data, topic, fs):
    """Streams data to a given Kafka topic. Publishes the data in one sample
    packets as a JSON map with two keys: 'data' for the content, and 'time'
    for a timestamp for the packet.

    Parameters
    ----------
    data : arr_like
        A list of JSON-serializable items.
    topic : Topic
        A pykafka Topic object for the target topic.
    fs : number
        The sampling frequency in Hz.
    """
    T = 1 / fs
    start_time = time.time()
    try:
        with topic.get_producer() as producer:
            t_next = time.time()
            for i in range(len(data)):
                # Store current time
                now = time.time()
                sent_on_time = False

                # Wait until it is time to send the data
                if now < t_next:
                    time.sleep(t_next - now)
                    sent_on_time = True   # Set to True when we need to wait to send data

                # Send data
                producer.produce(json.dumps({
                    'data': data[i].tolist(),
                    'time': t_next,
                }).encode())

                # Store next send time
                t_next += T

                if i % fs == 0:
                    print("[INFO] {} samples published in {:.2f} seconds".format(i, time.time() - start_time))
                if not sent_on_time:
                    print("[WARN] Publisher cannot keep up with data flow at sample {}, delay in seconds: {:.4f}.".format(i, abs(now-(t_next-T))))
    except (pykafka.exceptions.SocketDisconnectedError, pykafka.exceptions.LeaderNotAvailable) as e:
        sys.stderr.write("[ERROR] Error sending to Kafka. Message: '{}'.".format(e))
        producer.stop()
        sys.exit(1)

    end_time = time.time()
    print("[INFO] Sent {}x{} data in {:.2f} seconds.".format(*data.shape, end_time - start_time))


def main(datafiles, kafka_ip=None, kafka_port=None, kafka_topic=None):
    """Simple example of sending eeg data over Kafka.

    Parameters
    ----------
    datafiles : arr_like of str
        A list of file paths to send over Kafka.
    kafka_ip : str
        A valid IP address of the Kafka server.
    kafka_port : str or int
        The port of the Kafka server.
    kafka_topic : str
        The Kafka topic where to send the data.
    """

    # Set up Kafka client.
    print("[INFO] Setting up Kafka client connection")
    client = get_kafka_client(ip=kafka_ip, port=kafka_port)
    if client is None:
        sys.stderr.write("[ERROR] Kafka broker was not found. Make sure that Kafka is installed and running.\n")
        sys.exit(1)

    # Select Kafka topic
    topic = client.topics[(kafka_topic or os.getenv("KAFKA_TOPIC") or 'eeg_data')]

    # Read the data on at a time
    print("[INFO] Preloading data")
    raws = [read_raw_edf(f, preload=True) for f in datafiles]
    raw = concatenate_raws(raws)

    data, times = raw.get_data(return_times=True)
    data = np.transpose(data)
    fs = raw.info['sfreq']

    # TODO: Publish metadata (EEG channel names, sampling frequency,
    #   something else from raw.info?)

    print("[INFO] Starting to send data")
    stream_data(data, topic, fs)


if __name__ == "__main__":
    def ip(input_str):
        try:
            extracted_vals = input_str.split(".")
            if len(extracted_vals) != 4:
                raise argparse.ArgumentTypeError("IP address needs to be of form 'x.x.x.x'.")

            extracted_vals_int = [int(x) for x in extracted_vals]

            if min(extracted_vals_int) < 0 or max(extracted_vals_int) > 255:
                raise argparse.ArgumentTypeError("IP address needs to be of form 'x.x.x.x', where each x is 0..255.")

            return ".".join(str(x) for x in extracted_vals_int)

        except TypeError as e:
            raise argparse.ArgumentTypeError("Error parsing ip address.")

    parser = argparse.ArgumentParser(description = "Send EEG data over Kafka")
    parser.add_argument('--ip', type=ip, help="IP address of the Kafka server.")
    parser.add_argument('--port', type=int, help="Port of the Kafka server.")
    parser.add_argument('--topic', type=str, help="Kafka topic where to send the data.")
    parser.add_argument('filenames', type=str, help="Files to send over Kafka.", nargs='+')

    args = parser.parse_args()

    #datafile_paths = [os.path.join(os.getcwd(), path) for path in args.filenames]
    datafile_paths = args.filenames
    # Check for all files
    current_path = os.getcwd()
    active_path = current_path
    datafiles = list()
    for path in datafile_paths:
        if os.path.isfile(path):
            datafiles.append(os.path.realpath(path))
            active_path = os.path.dirname(os.path.abspath(path))
        elif os.path.isfile(os.path.join(current_path, path)):
            datafiles.append(os.path.abspath(os.path.join(current_path, path)))
        elif os.path.isfile(os.path.join(active_path, path)):
            datafiles.append(os.path.abspath(os.path.join(active_path, path)))
        else:
            sys.stderr.write("[ERROR] Could not find datafile '{}'. Aborting.\n".format(path))
            sys.exit(1)

    main(datafiles, kafka_ip=args.ip, kafka_port=args.port, kafka_topic=args.topic)
