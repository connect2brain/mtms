import json
import logging
import time
import csv
import queue
import threading
from multiprocessing import Process, Queue, Pool
from typing import Any, List

import pykafka
from pykafka.balancedconsumer import BalancedConsumer
from pykafka.simpleconsumer import SimpleConsumer
from pykafka.producer import Producer

from mtms.kafka.kafka import Kafka

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

class EegSimulator:
    """A class for sending simulated EEG data via Kafka.

    """

    def send_data(self, data_q: Queue, msg_q: Queue) -> None:
        """Asynchronously sends out a single piece of pre-formatted data.

        Parameters
        ----------
        data_q
            A queue where to read data from
        msg_q
            A queue where to write status messages etc.
        """
        threading.current_thread().name = "send_data"

        kafka = Kafka()

        producer: Producer = kafka.get_producer(topic='eeg_data')
        try:
            logging.info("send_data() ready to receive data.")

            count = 0
            while True:   # Run until the end of stream
                data = data_q.get()
                if data is None:
                    msg_q.put(time.time())
                    break
                msg_q.put(time.time())

                producer.produce(bytes(data))

                # Process delivery messages
                if count % 100 == 0:
                    success = 0
                    failed = 0
                    while True:
                        try:
                            msg, exc = producer.get_delivery_report(block=False)
                            if exc is not None:
                                failed += 1
                            else:
                                success += 1
                        except queue.Empty:
                            break
                    logging.info("Successfully delivered {} messages, failed to deliver {} messages.".format(success, failed))

                count += 1
        except (pykafka.exceptions.SocketDisconnectedError, pykafka.exceptions.LeaderNotAvailable) as e:
            logging.error("Error sending to Kafka. Message: '{}'.".format(e))
            producer.stop()

        logging.info("send_data() exiting.")
        msg_q.put(None)   # Send None to indicate the end to the main thread

    def tick_out_data(self,
            data: List[Any],
            time_interval: float,
            data_q: Queue,
            msg_q: Queue) -> None:
        """Ticks out data with given time interval.

        Parameters
        ----------
        data
            A list of JSON-serializable items.
        time_interval
            Time interval between data sends.
        data_q
            Queue where to send data to.
        msg_q
            Queue where to send status messages.
        """
        threading.current_thread().name = "tick_out_data"

        logging.info("Ticking out data every {:.2f} ms.".format(1000.0*time_interval))
        logging.info("tick_out_data() starting to send data.")

        next_t = time.time()

        for row in range(len(data)):
            data_str = json.dumps({
                        'data': data[row].tolist(),
                        'time': next_t,
                    }).encode()

            time.sleep(max(next_t - time.time(), 0))   # Wait for next send time
            msg_q.put(time.time())
            data_q.put(data_str)

            next_t += time_interval

        logging.info("tick_out_data() exiting.")
        msg_q.put(None)   # Send None to indicate the end to the main thread
        data_q.put(None)   # Send None to indicate the end to the sender


    def receive_data(self, msg_q: Queue) -> None:
        """Listens for data from the given Kafka topic.

        Parameters
        ----------
        msg_q
            A queue where to write status messages etc.
        """
        threading.current_thread().name = "receive_data"

        kafka = Kafka()

        # TODO: BalancedConsumer was used here previously, but it didn't work.
        #       Change it back and fix?
        consumer: SimpleConsumer = kafka.get_consumer(
            topic='eeg_data',
        )

        logging.info("receive_data() starting to listen for data.")
        try:
            consumer.start()
            # TODO: Add a mechanism for breaking this loop once all messages have been
            #       received. That will probably need re-thinking the content of Kafka
            #       topics, as the current 'eeg_data' only serves to transmit raw data
            #       and not metadata, such as the end of stream.
            while True:
                #msg = consumer.consume()
                for msg in consumer:
                    if msg is not None:
                        msg_q.put(time.time())

            consumer.commit_offsets()
            consumer.stop()

        except pykafka.exceptions.ConsumerStoppedException as e:
            logging.error("Kafka consumer stopped unexpectedly.")
            msg_q.put(None)

        logging.info("receive_data() exiting.")
        msg_q.put(None)   # Send None to indicate the end

    def stream_data_mp(self,
            data: List[Any],
            fs: int,
            store_data: bool = False) -> bool:
        """This is a multiprocessing version of stream_data.
        Streams data to a given Kafka topic. Publishes the data in one sample
        packets as a JSON map with two keys: 'data' for the content, and 'time'
        for a timestamp for the packet.

        Parameters
        ----------
        data
            A list of JSON-serializable items.
        fs
            The sampling frequency in Hz.
        store_data
            A boolean indicating if the timing data from the streaming is stored.

        Returns
        -------
            A boolean indicating if the streaming was successful.
        """
        T = 1 / fs

        data_q = Queue()
        listener_msg_q = Queue()
        sender_msg_q = Queue()
        ticker_msg_q = Queue()
        listener = Process(target=self.receive_data, args=(listener_msg_q, ))
        sender = Process(target=self.send_data, args=(data_q, sender_msg_q))
        ticker = Process(target=self.tick_out_data, args=(data, T, data_q, ticker_msg_q))

        try:
            listener.start()
            sender.start()
            ticker.start()

            start_time = time.time()

            listener_msgs = []
            ticker_msgs = []
            sender_msgs = []

            logging.info("Started")
            ticker_ended = False
            sender_ended = False
            listener_ended = False
            timeout_time = 0.25
            cntr = 0
            prev = time.time()

            # TODO: The current stopping condition for the streaming is that all messages
            # have been sent to Kafka. This likely causes some messages to not have been
            # received by the listener by the time we get out of this loop, but that's the
            # best that we can do robustly as long as there is no explicit "end of stream"
            # message in Kafka. Fix this later.
            while not sender_ended:
                listener_timed_out = False
                sender_timed_out = False
                ticker_timed_out = False

                now = time.time()

                if now-prev > 0.5:
                    mdiff = len(ticker_msgs)-len(sender_msgs)
                    logging.info('{}'.format('+' if mdiff > 0 else '-' if mdiff < 0 else '.'))
                    prev = now

                if not ticker_ended:
                    try:
                        msg = ticker_msg_q.get(block=False)
                        if msg is None:
                            ticker_ended = True
                        else:
                            ticker_msgs.append(msg)
                    except queue.Empty:
                        ticker_timed_out = True

                if not sender_ended:
                    try:
                        msg = sender_msg_q.get(block=False)
                        if msg is None:
                            sender_ended = True
                        else:
                            sender_msgs.append(msg)
                    except queue.Empty:
                        sender_timed_out = True

                # XXX: Currently, the listener doesn't receive the information about the
                #      end of stream, therefore listener_ended is semantically different
                #      from sender_ended and ticker_ended: in contrast to the latter two,
                #      listener_ended is set to True only when something fails. Change
                #      the semantics to match with each other.
                if not listener_ended:
                    try:
                        msg = listener_msg_q.get(block=False)
                        if msg is None:
                            listener_ended = True
                        else:
                            listener_msgs.append(msg)
                    except queue.Empty:
                        listener_timed_out = True

                if ticker_timed_out and sender_timed_out and listener_timed_out:
                    time.sleep(timeout_time)

        except KeyboardInterrupt as e:
            logging.info("Stopped.")
            logging.info("Keyboard interrupt received. Stopping child processes.")
            ticker.terminate()
            sender.terminate()
            listener.terminate()

        else:
            logging.info("Ended.")
            sender.terminate()
            ticker.terminate()
            listener.terminate()
        
        finally:
            end_time = time.time()


        logging.info("Sent {}x{} data in {:.2f} seconds.".format(*data.shape, end_time - start_time))
        logging.info("Received {} status messages from ticker.".format(len(ticker_msgs)))
        logging.info("Received {} status messages from sender.".format(len(sender_msgs)))
        logging.info("Received {} status messages from listener.".format(len(listener_msgs)))

        if len(ticker_msgs) > 0:
            logging.info("It took {:.2f} s to tick out data.".format(ticker_msgs[-1]-ticker_msgs[0]))
        if len(sender_msgs) > 0:
            logging.info("It took {:.2f} s to send data.".format(sender_msgs[-1]-sender_msgs[0]))
        if len(listener_msgs) > 0:
            logging.info("It took {:.2f} s to receive data.".format(listener_msgs[-1]-listener_msgs[0]))
        
        if len(ticker_msgs) > 0 and len(sender_msgs) > 0:
            logging.info("First packet was sent {:.2f} ms later than ticked out.".format(1000*(sender_msgs[0]-ticker_msgs[0])))
            logging.info("Last packet was sent {:.2f} ms later than ticked out.".format(1000*(sender_msgs[-1]-ticker_msgs[-1])))
        else:
            logging.info("Not enough ticker and/or sender messages.")
        if len(sender_msgs) > 0 and len(listener_msgs) > 0:
            logging.info("First packet was received {:.2f} ms later than sent out.".format(1000*(listener_msgs[0]-sender_msgs[0])))
            logging.info("Last packet was received {:.2f} ms later than sent out.".format(1000*(listener_msgs[-1]-sender_msgs[-1])))
        else:
            logging.info("Not enough sender or listener messages.")

        if len(ticker_msgs) > 0 and len(sender_msgs) > 0:
            dts = [(y - x) for x, y in zip(ticker_msgs, sender_msgs)]
            logging.info("Average time delay between ticking and sending was {:.2f} ms.".format(1000*sum(dts)/len(dts)))
            logging.info("Maximum time delay between ticking and sending was {:.2f} ms.".format(1000*max(dts)))
            logging.info("Minimum time delay between ticking and sending was {:.2f} ms.".format(1000*min(dts)))
        else:
            logging.info("Not enough ticker and/or sender messages.")

        if len(sender_msgs) > 0 and len(listener_msgs) > 0:
            dts2 = [(y - x) for x, y in zip(sender_msgs, listener_msgs)]
            logging.info("Average time delay between sending and receiving was {:.2f} ms.".format(1000*sum(dts2)/len(dts2)))
            logging.info("Maximum time delay between sending and receiving was {:.2f} ms.".format(1000*max(dts2)))
            logging.info("Minimum time delay between sending and receiving was {:.2f} ms.".format(1000*min(dts2)))
        else:
            logging.info("Not enough sender and/or listener messages.")

        if store_data:
            if len(ticker_msgs) > 0 and len(sender_msgs) > 0 and len(listener_msgs) > 0:
                logging.info("Saving data to output/timing.csv.")
                with open("output/timing.csv", "w", newline="") as f:
                    writer = csv.writer(f)
                    writer.writerows(list(zip(ticker_msgs, sender_msgs, listener_msgs)))
            else:
                logging.info("Not enough data. Skipping save.")

        # TODO: A couple of conditions for determining if the streaming was successful,
        #       not exhaustive yet.
        if len(ticker_msgs) == 0:
            logging.error("No messages were ticked for sending.")
            return False

        if len(sender_msgs) == 0:
            logging.error("No messages were sent to Kafka.")
            return False

        if len(listener_msgs) == 0:
            logging.error("No messages were received from Kafka.")
            return False

        return True
