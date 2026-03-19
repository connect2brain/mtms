# NeurOne Simulator (C++ CLI)

This folder contains a C++ version of the NeurOne EEG simulator.

It sends NeurOne-like UDP packets (`measurement_start`, `samples`, `measurement_end`) and listens for TCP trigger messages (`trigger_a`, `trigger_b`) on localhost.

## Build

From this directory:

```bash
g++ -std=c++17 -O3 -pthread neurone_simulator.cpp -o neurone_simulator
```

## Run

```bash
./neurone_simulator
```

Run with a fixed duration:

```bash
./neurone_simulator --duration 10
```

Example with custom ports and channels:

```bash
./neurone_simulator \
  --port 50000 \
  --trigger-port 60000 \
  --sampling-rate 5000 \
  --eeg-channels 63 \
  --emg-channels 10
```

Show CLI options:

```bash
./neurone_simulator --help
```

## Trigger Testing

In another terminal, send triggers over TCP:

```bash
printf "trigger_a\n" | nc 127.0.0.1 60000
printf "trigger_b\n" | nc 127.0.0.1 60000
```

## Notes

- `trigger_a` can be delayed with `--trigger-a-delay`.
- `trigger_b` can be disabled with `--disable-trigger-b`.
- Dropped samples can be simulated with `--simulate-dropped-samples`.
