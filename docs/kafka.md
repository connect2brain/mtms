# Apache Kafka

## General

We use Apache Kafka for several purposes:

- to pass parameters from the user to the system,

- to pass state of the system to the user,

- to stream data, e.g., EEG data, through the system.

## Topics

Here is a list of Kafka topics, published by different components of the system,
divided into three categories: parameters, commands, and state.

### Parameters

The parameters are numeric values, set by the user before the stimulation starts, to
control various aspects of the stimulation.

The latest published value in the topic is treated as the current value of the parameter.

#### intensity (integer)

The intensity of stimulation in millivolts.

Range: 1–1000

#### iti (integer)

The inter-trial interval in milliseconds.

Range: 1–5000

#### number_of_stimuli (integer)

The number of stimuli in the stimulation.

Range: 1–100

### Commands

The commands are used to trigger changes in the operation of the system.

While the action that the command triggers is being carried out, further commands
triggering either the same action or a different action are ignored.

The exception is 'abort' command, which aborts the current action if there is
an ongoing action, and otherwise does nothing.

'Abort' is idempotent, i.e., sending several consequent 'abort' commands does not have
further effects.

The commands are passed in Kafka topics of the same name, by publishing the
value '1' in the topic.

A command only has an effect if the stimulation software is listening to the topic at
the time of publishing the topic. It follows from this that the latest published
command does not have a meaning in itself, and neither does the latest published
command of a given name (e.g., 'stimulate').

A command, when listened to by the stimulation software, should trigger a change in
the system state.

#### recharge

Recharge the capacitors. 'recharging' state topic is used to indicate if recharging
is underway.

'Abort' command aborts recharging.

#### stimulate

Start stimulation with the current parameters. 'stimulating' state topic is used to
indicate if stimulation is ongoing.

Only has an effect if 'ready_to_stimulate' state topic is False.

'Abort' command aborts stimulation.

#### abort

Abort stimulation or recharging if ongoing, otherwise do nothing.

### State

State topics are used by the stimulation software to indicate the current state of the system.

The latest published value in the topic is treated as the current state.

Boolean topics have one of the values 'True' or 'False'.

#### charging (boolean)

Indicates if charging the capacitors is underway.

Set to True when 'recharge' command is received.

Set to False when recharging is finished or aborted.

#### stimulating (boolean)

Indicates if stimulation is ongoing.

Set to True when 'stimulate' command is received.

Set to False when stimulation is finished or aborted.

When True, parameter changes do not have an effect.

#### ready_to_stimulate (boolean)

Indicates if the system is ready to receive a 'stimulate' command.

A necessary condition for this to be True is that 'stimulating' is False and
'charging' is False.

#### charging_times (integer array)

An array (1 x transducer channel count) consisting of the times that it took to charge the
capacitors.

#### train_count (integer)

A number indicating how many pulse trains have been given in an ongoing stimulation.

0 if stimulation is not ongoing, otherwise ranges from 0 to [??].

#### error_code (integer)

An error code indicating the error state of the system.

0 if there is no error in the system.

#### sequence_counter (integer)

TBD.

#### sequence_length (integer)

TBD.

### Data

The data topics consist of measurements done by the system during its operation.

#### eeg_data (float array)

An array (1 x EEG channel count) consisting of the measured EEG values in each channel.
