# Databases

## Topics

### General

The project uses a database for storing topic-related metadata.

The database is created when by running the make target:

    make init-db

The database is created as `db/topics.db`.

The database is read-only, ensuring that it is immutable.

### Database structure

The database consists of the table `topics`, with the columns `name`, `type`, `activex_control_name`,
and `latch`.

#### Name

The name of the topic, e.g., `eeg_data`.

#### Type

Either `parameter` or `stream`.

- `parameter` is used for topics that contain the stimulation parameters, which are not controlled in
real-time.
- `stream` is used for topics that contain data streamed through the system, such as EEG data.

#### Activex_control_name

Defined for topics of `parameter` type, which are passed to mTMS software. Contains the name of the ActiveX
control for setting the parameter value in mTMS software.

#### Latch

Either `0` or `1`, `0` indicating that latching is disabled for the topic, and `1` indicating that it is enabled.

If latching is enabled, a new consumer of the topic will receive the latest message produced into the topic. If
disabled, the consumer will only receive messages produced after the consumer was created.

It is recommended to enable latching for `parameter` topics.
