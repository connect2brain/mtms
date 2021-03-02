# Databases

## Topics

### General

The project uses an SQLite database for storing topic-related, immutable metadata.

The database is created by running the script `init-db.sh`.

The database is created as `db/topics.db`.

The database is read-only, ensuring that it is immutable.

### Database structure

The database consists of the table `topics`, with the columns `name`, `type`, and `latch`.

#### Name

The name of the topic, e.g., `eeg_data`.

#### Type

Either `parameter`, `command`, `state`, or `stream`.

- `parameter` is used for topics that contain the stimulation parameters, which are not controlled in
real-time.
- `command` is used for topics that pass commands from the user interface to the stimulation software.
- `state` is used for topics that pass system state from the stimulation software back to Kafka and further
to other components of the system.
- `stream` is used for topics that contain data streamed through the system, such as EEG data.

#### Latch

Either `0` or `1`, `0` indicating that latching is disabled for the topic, and `1` indicating that it is enabled.

If latching is enabled, a new consumer of the topic will receive the latest message produced into the topic. If
disabled, the consumer will only receive messages produced after the consumer was created.

Latching should be enabled for `parameter` topics and potentially for `state` topics.
