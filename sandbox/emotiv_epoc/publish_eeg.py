from lib.cortex import Cortex
from pykafka import KafkaClient
import json

url = "wss://localhost:6868"

# Data specific to my subscription of the API.

user = {
	"license" : "5aa807cb-e69a-4d2a-8ec2-c5e0f3d63042",
	"client_id" : "7IflwPckETcmfULmYJ95i8qN2uIeVhWLie4BpHom",
	"client_secret" : "BQI5fyQFukcinepmZYE8yEk6sGQRHKUq9shK4TacxGSv0qUkuoD8oLF2AO3vccOaO5joLXnSqVJFcmW1ej0y9xpZMO5QxTxlkniiDZrlwhg7XHy2YenhSL9sdgV4UFNN",
	"debit" : 100,
	"number_row_data" : 10
}

# Initialize Cortex API.

c = Cortex(url, user)

print('access device\n')
c.grant_access_and_session_info()

print('subscribe to EEG stream\n')
streams = ['eeg']
info = c.subRequest(streams)
print(info)

print('receive rows\n')
data = c.receiveRow()
print(data)

# Set up Kafka client.

client = KafkaClient(hosts="127.0.0.1:9092")
topic = client.topics['eeg_raw']

with topic.get_sync_producer() as producer:
	while True:
		row = c.receiveRow()
		print(row)
		producer.produce(row.encode())
