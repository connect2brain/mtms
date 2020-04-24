import argparse
import json
import os
import sys
import time
import csv

import dotenv
from mne.io import concatenate_raws, read_raw_edf
import numpy as np
import pykafka
from pykafka import KafkaClient

from multiprocessing import Process, Queue, Pool
import queue

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


def send_data(topic, data_q, msg_q):
    """Asynchronously sends out a single piece of pre-formatted data.

    Parameters
    ----------
    topic : str_like
        Kafka topic to use
    data_q : multiprocessing.Queue
        A queue where to read data from
    msg_q : multiprocessing.Queue
        A queue where to write status messages etc.
    """

    try:
        with topic.get_producer() as producer:
            sys.stdout.write("[INFO] send_data() ready to receive data.\n")

            while True:   # Run until stopped
                data = data_q.get()
                if data is None:
                    msg_q.put(time.time())
                    break
                msg_q.put(time.time())
                producer.produce(data)
    except (pykafka.exceptions.SocketDisconnectedError, pykafka.exceptions.LeaderNotAvailable) as e:
        sys.stderr.write("[ERROR] Error sending to Kafka. Message: '{}'.\n".format(e))
        producer.stop()

    sys.stdout.write("[INFO] send_data() exiting.\n")


def tick_out_data(data, time_interval, data_q, msg_q):
    """Ticks out data with given time interval.

    Parameters
    ----------
    data : arr_like
        A list of JSON-serializable items.
    time_interval : float
        Time interval between data sends.
    data_q : multiprocessing.Queue
        Queue where to send data to.
    msg_q : multiprocessing.Queue
        Queue where to send status messages.
    """
    sys.stdout.write("[INFO] Ticking out data every {:.2f} ms.\n".format(1000.0*time_interval))
    sys.stdout.write("[INFO] tick_out_data() starting to send data.\n")

    start_time = time.time()
    next_t = start_time

    for row in range(len(data)):
        data_str = json.dumps({
                    'data': data[row].tolist(),
                    'time': next_t - start_time,
                }).encode()

        time.sleep(min(next_t - time.time()), 0)   # Wait for next send time
        msg_q.put(time.time())
        data_q.put(data_str)

        next_t += time_interval

    sys.stdout.write("[INFO] tick_out_data() exiting.\n")



def stream_data_mp(data, topic, fs, senders=1):
    """This is a multiprocessing version of stream_data.
    Streams data to a given Kafka topic. Publishes the data in one sample
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

    data_q = Queue()
    sender_msg_q = Queue()
    ticker_msg_q = Queue()
    sender = Process(target=send_data, args=(topic, data_q, sender_msg_q))
    ticker = Process(target=tick_out_data, args=(data, T, data_q, ticker_msg_q))

    sender.start()
    ticker.start()

    start_time = time.time()

    ticker_msgs = []
    sender_msgs = []

    sys.stdout.write("Started")
    sys.stdout.flush()
    timed_out_times = 10
    cntr = 0
    prev = time.time()

    while timed_out_times > 0:
        now = time.time()
        timed_out = 2
        
        if now-prev > 1.0:
            sys.stdout.write('|{}'.format(len(ticker_msgs)-len(sender_msgs)))
            sys.stdout.flush()

        try:
            ticker_msgs.append(ticker_msg_q.get(True, timeout=T))
        except queue.Empty:
            timed_out -= 1

        try:
            sender_msgs.append(sender_msg_q.get(True, timeout=T))
        except queue.Empty:
            timed_out -= 1

        if timed_out == 0:
            timed_out_times -= 1
        else:
            timed_out_times = 10

        prev = now

    sys.stdout.write("|Ended.\n")
    end_time = time.time()
    
    sender.terminate()
    ticker.terminate()

    print("[INFO] Sent {}x{} data in {:.2f} seconds.".format(*data.shape, end_time - start_time))
    print("[INFO] Received {} status messages from ticker.".format(len(ticker_msgs)))
    print("[INFO] Received {} status messages from sender.".format(len(sender_msgs)))
    print("[INFO] It took {:.2f} s to tick out data.".format(ticker_msgs[-1]-ticker_msgs[0]))
    print("[INFO] It took {:.2f} s to send data.".format(sender_msgs[-1]-sender_msgs[0]))
    print("[INFO] First packet was sent {:.2f} ms later than ticked out.".format(1000*(sender_msgs[0]-ticker_msgs[0])))
    print("[INFO] Last packet was sent {:.2f} ms later than ticked out.".format(1000*(sender_msgs[-1]-ticker_msgs[-1])))
    dts = [(y - x) for x, y in zip(ticker_msgs, sender_msgs)]
    print("[INFO] Average time delay between ticking and sending was {:.2f} ms.".format(1000*sum(dts)/len(dts)))
    print("[INFO] Maximum time delay between ticking and sending was {:.2f} ms.".format(1000*max(dts)))
    print("[INFO] Minimum time delay between ticking and sending was {:.2f} ms.".format(1000*min(dts)))

    print("[INFO] Saving data to timing.csv.")
    with open("timing.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(list(zip(ticker_msgs, sender_msgs)))


    #print("[INFO] {} packets out of {} were sent late.".format(sum([1 for x in times_log if x < 0]), len(times_log)))
    #print("[INFO] On average, packet was sent {:.1f} ms late.".format(1000*sum([-x for x in times_log if x < 0])/sum([1 for x in times_log if x < 0])))


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
    print(f"[INFO] Frequency = {fs}.")
    print(f"[INFO] Cycle time = {1.0/fs}.")

    # TODO: Publish metadata (EEG channel names, sampling frequency,
    #   something else from raw.info?)

    print("[INFO] Starting to send data")
    stream_data_mp(data, topic, fs)


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
