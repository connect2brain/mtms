import json
import logging
import time
import csv
import queue
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
        kafka = Kafka()

        producer: Producer = kafka.get_producer(topic='eeg_data')
        try:
            logging.info("send_data() ready to receive data.")

            count = 0
            while True:   # Run until stopped
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
                                #logging.info("Failed to deliver msg {}: {}".format(
                                #    msg.partition_key, repr(exc)))
                                failed += 1
                            else:
                                #logging.info("Successfully delivered msg {}".format(
                                #    msg.partition_key))
                                success += 1
                        except queue.Empty:
                            break
                        finally:
                            logging.info("S{}F{}".format(success, failed))
                count += 1
        except (pykafka.exceptions.SocketDisconnectedError, pykafka.exceptions.LeaderNotAvailable) as e:
            logging.error("Error sending to Kafka. Message: '{}'.".format(e))
            producer.stop()

        logging.info("send_data() exiting.")
        msg_q.put(None)   # Send None to indicate the end

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
        msg_q.put(None)   # Send None to indicate the end


    def receive_data(self, msg_q: Queue) -> None:
        """Listens for data from the given Kafka topic.

        Parameters
        ----------
        msg_q
            A queue where to write status messages etc.
        """
        kafka = Kafka()

        consumer: BalancedConsumer = kafka.get_balanced_consumer(
            topic='eeg_data',
            consumer_group=b"benchmark",
            auto_commit_enable=True,
            auto_offset_reset=pykafka.common.OffsetType.LATEST,
            reset_offset_on_start=True,
        )

        logging.info("receive_data() starting to listen for data.")
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
            logging.error("Kafka consumer stopped unexpectedly.")
            msg_q.put(None)

        logging.info("receive_data() exiting.")
        msg_q.put(None)   # Send None to indicate the end

    def stream_data_mp(self, data: List[Any], fs: int, senders: int = 1) -> None:
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
            listener_timed_out = False
            timeout_time = 0.25
            cntr = 0
            prev = time.time()

            while not ticker_ended or not sender_ended or not listener_timed_out or not listener_ended:
                now = time.time()

                if now-prev > 0.5:
                    mdiff = len(ticker_msgs)-len(sender_msgs)
                    logging.info('{}'.format('+' if mdiff > 0 else '-' if mdiff < 0 else '.'))
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
            logging.info("|Stopped.\n")
            logging.info("Keyboard interrupt received. Stopping child processes.")
            ticker.terminate()
            sender.terminate()
            listener.terminate()

        else:
            logging.info("|Ended.\n")
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

        if len(ticker_msgs) > 0 and len(sender_msgs) > 0 and len(listener_msgs) > 0:
            logging.info("Saving data to output/timing.csv.")
            with open("output/timing.csv", "w", newline="") as f:
                writer = csv.writer(f)
                writer.writerows(list(zip(ticker_msgs, sender_msgs, listener_msgs)))
        else:
            logging.info("Not enough data. Skipping save.")

        #logging.info("{} packets out of {} were sent late.".format(sum([1 for x in times_log if x < 0]), len(times_log)))
        #logging.info("On average, packet was sent {:.1f} ms late.".format(1000*sum([-x for x in times_log if x < 0])/sum([1 for x in times_log if x < 0])))
