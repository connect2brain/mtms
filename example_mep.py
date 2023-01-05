# TODO: Only works with simulated EEG data, as EEG bridge does not
#   publish data unless an experiment is ongoing. Extend to cover the
#   real-data case, as well.

from mtms_api.MTMSApi import MTMSApi

api = MTMSApi()

amplitude, latency = api.analyze_mep(
    time=2.0,
    emg_channel=0,
)

print(amplitude, latency)

api.end()
