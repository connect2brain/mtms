import json
from mne.io import concatenate_raws, read_raw_edf
from pykafka import KafkaClient

# Read the data.

# TODO: Let user specify dataset names and locations from command line.
#   Should it support streaming several datasets or only one for each call
#   of the script?
raw_fnames = ['example-data/S001R01.edf', 'example-data/S001R02.edf']

raws = [read_raw_edf(f, preload=True) for f in raw_fnames]
raw = concatenate_raws(raws)

data, times = raw.get_data(return_times=True)
n_chs, n_timepoints = data.shape

# Set up Kafka client.

# TODO: Let user specify host and topic name from command line.
client = KafkaClient(hosts="127.0.0.1:9092")
topic = client.topics['eeg_data']

with topic.get_sync_producer() as producer:
    i = 0
    while i < n_timepoints:
        # TODO: Instead of publishing data as fast as possible, simulate the
        #   time properties of the original recording.
        #
        # TODO: Publish metadata (EEG channel names, sampling frequency,
        #   something else from raw.info?)
        row = data[:, i]
        producer.produce(json.dumps(row.tolist()).encode())
        i = i + 1
