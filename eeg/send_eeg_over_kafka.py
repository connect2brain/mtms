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


def get_kafka_client(ip=None, port=None, zookeeper_hosts=None, use_greenlets=False):
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
            port=(port or os.getenv("KAFKA_PORT") or '9092'),
            zookeeper_hosts=(zookeeper_hosts or os.getenv("ZOOKEEPER_HOSTS")),
            use_greenlets=use_greenlets))
        return client
    except pykafka.exceptions.NoBrokersAvailableError as e:
        return None


def send_data(data_q, msg_q, print_delivery_reports=True, linger_ms=0, use_rdkafka=True):
    """Asynchronously sends out a single piece of pre-formatted data.

    Parameters
    ----------
    data_q : multiprocessing.Queue
        A queue where to read data from
    msg_q : multiprocessing.Queue
        A queue where to write status messages etc.
    print_delivery_reports : Bool
        If True a summary of delivery reports is written to stdout
    """
    # Set up Kafka client.
    sys.stdout.write("[INFO] send_data(): setting up Kafka client connection.\n")
    client = get_kafka_client()
    if client is None:
        sys.stderr.write("[ERROR] Kafka broker was not found. Make sure that Kafka is installed and running.\n")
        msg_q.put(None)   # Send None to indicate the end
        return

    topic = client.topics[os.getenv("KAFKA_TOPIC") or 'eeg_data']
 
    try:
        with topic.get_producer(sync=False, 
            delivery_reports=print_delivery_reports, 
            linger_ms=linger_ms,
            use_rdkafka=use_rdkafka) as producer:
            sys.stdout.write("[INFO] send_data() ready to receive data.\n")

            count = 0
            while True:   # Run until stopped
                data = data_q.get()
                if data is None:
                    msg_q.put(time.time())
                    break
                msg_q.put(time.time())
                #producer.produce(data)
                producer.produce(bytes(data))

                if print_delivery_reports:
                    # Process delivery messages
                    if count % 100 == 0:
                        success = 0
                        failed = 0
                        while True:
                            try:
                                msg, exc = producer.get_delivery_report(block=False)
                                if exc is not None:
                                    #sys.stdout.write("Failed to deliver msg {}: {}".format(
                                    #    msg.partition_key, repr(exc)))
                                    failed += 1
                                else:
                                    #sys.stdout.write("Successfully delivered msg {}".format(
                                    #    msg.partition_key))
                                    success += 1
                            except queue.Empty:
                                break
                            finally:
                                sys.stdout.write("S{}F{}".format(success, failed))

                count += 1
    except (pykafka.exceptions.SocketDisconnectedError, pykafka.exceptions.LeaderNotAvailable) as e:
        sys.stderr.write("[ERROR] Error sending to Kafka. Message: '{}'.\n".format(e))
        producer.stop()

    sys.stdout.write("[INFO] send_data() exiting.\n")
    msg_q.put(None)   # Send None to indicate the end


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

        time.sleep(max(next_t - time.time(), 0))   # Wait for next send time
        msg_q.put(time.time())
        data_q.put(data_str)

        next_t += time_interval

    sys.stdout.write("[INFO] tick_out_data() exiting.\n")
    msg_q.put(None)   # Send None to indicate the end


def receive_data(msg_q, use_rdkafka=True):
    """Listens for data from the given Kafka topic.

    Parameters
    ----------
    msg_q : multiprocessing.Queue
        A queue where to write status messages etc.
    """
    # Set up Kafka client.
    sys.stdout.write("[INFO] receive_data(): setting up Kafka client connection.\n")
    client = get_kafka_client()
    if client is None:
        sys.stderr.write("[ERROR] Kafka broker was not found. Make sure that Kafka is installed and running.\n")
        msg_q.put(None)   # Send None to indicate the end
        return

    # Select Kafka topic
    topic = client.topics[(os.getenv("KAFKA_TOPIC") or 'eeg_data')]
    offset = topic.latest_available_offsets()

    sys.stdout.write("[INFO] receive_data() starting to listen for data.\n")
    consumer = topic.get_balanced_consumer(
        consumer_group=b"benchmark",
        auto_commit_enable=True,
        auto_offset_reset=pykafka.common.OffsetType.LATEST,
        reset_offset_on_start=True,
        use_rdkafka=use_rdkafka)

    try:
        consumer.start()
        while True:
            #msg = consumer.consume()
            for msg in consumer:
                if msg is not None:
                    msg_q.put(time.time())
            
        consumer.commit_offsets()
        consumer.stop()

    except pykafka.exceptions.ConsumerStoppedException as e:
        sys.stderr.write("[ERROR] Kafka consumer stopped unexpectedly.")
        msg_q.put(None)

    sys.stdout.write("[INFO] receive_data() exiting.\n")
    msg_q.put(None)   # Send None to indicate the end


def stream_data_mp(data, fs, senders=1):
    """This is a multiprocessing version of stream_data.
    Streams data to a given Kafka topic. Publishes the data in one sample
    packets as a JSON map with two keys: 'data' for the content, and 'time'
    for a timestamp for the packet.

    Parameters
    ----------
    data : arr_like
        A list of JSON-serializable items.
    fs : number
        The sampling frequency in Hz.
    """
    T = 1 / fs

    data_q = Queue()
    listener_msg_q = Queue()
    sender_msg_q = Queue()
    ticker_msg_q = Queue()
    listener = Process(target=receive_data, args=(listener_msg_q, ), kwargs={"use_rdkafka": True})
    sender = Process(target=send_data, args=(data_q, sender_msg_q), kwargs={"use_rdkafka": True})
    ticker = Process(target=tick_out_data, args=(data, T, data_q, ticker_msg_q))

    try:
        listener.start()
        sender.start()
        ticker.start()

        start_time = time.time()

        listener_msgs = []
        ticker_msgs = []
        sender_msgs = []

        sys.stdout.write("Started")
        sys.stdout.flush()
        ticker_ended = False
        sender_ended = False
        listener_ended = False
        listener_timed_out = False
        timeout_time = 0.25
        cntr = 0
        prev = time.time()

        while ((not ticker_ended) or (not sender_ended) or (not listener_timed_out or not listener_ended)):
            now = time.time()
            
            if now-prev > 0.5:
                mdiff = len(ticker_msgs)-len(sender_msgs)
                sys.stdout.write('{}'.format('+' if mdiff > 0 else '-' if mdiff < 0 else '.'))
                sys.stdout.flush()
                prev = now

            if not ticker_ended:
                try:
                    msg = ticker_msg_q.get(True, timeout=timeout_time)
                    if msg is None:
                        #ticker_ended = True
                        pass
                    else:
                        ticker_msgs.append(msg)
                except queue.Empty:
                    pass

            if not sender_ended:
                try:
                    msg = sender_msg_q.get(True, timeout=timeout_time)
                    if msg is None:
                        #sender_ended = True
                        pass
                    else:
                        sender_msgs.append(msg)
                except queue.Empty:
                    pass

            try:
                msg = listener_msg_q.get(True, timeout=timeout_time)
                if msg is None:
                    pass
                    #listener_ended = True
                else:
                    listener_msgs.append(msg)
                    listener_timed_out = False
            except queue.Empty:
                listener_timed_out = True
        
    except KeyboardInterrupt as e:
        sys.stdout.write("|Stopped.\n")
        print("[INFO] Keyboard interrupt received. Stopping child processes.")
        ticker.terminate()
        sender.terminate()
        listener.terminate()

    else:
        sys.stdout.write("|Ended.\n")
        sender.terminate()
        ticker.terminate()
        listener.terminate()
    
    finally:
        end_time = time.time()


    print("[INFO] Sent {}x{} data in {:.2f} seconds.".format(*data.shape, end_time - start_time))
    print("[INFO] Received {} status messages from ticker.".format(len(ticker_msgs)))
    print("[INFO] Received {} status messages from sender.".format(len(sender_msgs)))
    print("[INFO] Received {} status messages from listener.".format(len(listener_msgs)))

    if (len(ticker_msgs) > 0):
        print("[INFO] It took {:.2f} s to tick out data.".format(ticker_msgs[-1]-ticker_msgs[0]))
    if (len(sender_msgs) > 0):
        print("[INFO] It took {:.2f} s to send data.".format(sender_msgs[-1]-sender_msgs[0]))
    if (len(listener_msgs) > 0):
        print("[INFO] It took {:.2f} s to receive data.".format(listener_msgs[-1]-listener_msgs[0]))
    
    if (len(ticker_msgs) > 0 and len(sender_msgs) > 0):
        print("[INFO] First packet was sent {:.2f} ms later than ticked out.".format(1000*(sender_msgs[0]-ticker_msgs[0])))
        print("[INFO] Last packet was sent {:.2f} ms later than ticked out.".format(1000*(sender_msgs[-1]-ticker_msgs[-1])))
    else:
        print("[INFO] Not enough ticker and/or sender messages.")
    if (len(sender_msgs) > 0 and len(listener_msgs) > 0):
        print("[INFO] First packet was received {:.2f} ms later than sent out.".format(1000*(listener_msgs[0]-sender_msgs[0])))
        print("[INFO] Last packet was received {:.2f} ms later than sent out.".format(1000*(listener_msgs[-1]-sender_msgs[-1])))
    else:
        print("[INFO] Not enough sender or listener messages.")

    if (len(ticker_msgs) > 0 and len(sender_msgs) > 0):
        dts = [(y - x) for x, y in zip(ticker_msgs, sender_msgs)]
        print("[INFO] Average time delay between ticking and sending was {:.2f} ms.".format(1000*sum(dts)/len(dts)))
        print("[INFO] Maximum time delay between ticking and sending was {:.2f} ms.".format(1000*max(dts)))
        print("[INFO] Minimum time delay between ticking and sending was {:.2f} ms.".format(1000*min(dts)))
    else:
        print("[INFO] Not enough ticker and/or sender messages.")

    if (len(sender_msgs) > 0 and len(listener_msgs) > 0):
        dts2 = [(y - x) for x, y in zip(sender_msgs, listener_msgs)]
        print("[INFO] Average time delay between sending and receiving was {:.2f} ms.".format(1000*sum(dts2)/len(dts2)))
        print("[INFO] Maximum time delay between sending and receiving was {:.2f} ms.".format(1000*max(dts2)))
        print("[INFO] Minimum time delay between sending and receiving was {:.2f} ms.".format(1000*min(dts2)))
    else:
        print("[INFO] Not enough sender and/or listener messages.")


    if (len(ticker_msgs) > 0 and len(sender_msgs) > 0 and len(listener_msgs) > 0):
        print("[INFO] Saving data to timing.csv.")
        with open("timing.csv", "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerows(list(zip(ticker_msgs, sender_msgs, listener_msgs)))
    else:
        print("[INFO] Not enough data. Skipping save.")


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
    stream_data_mp(data, fs)


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
