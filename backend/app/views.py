import sys
import json
import time

from app import app
from flask import current_app, request

# TODO: Document the API endpoint
@app.route('/eeg_data')
def get_eeg_data():
    args_from = float(request.args.get('from', -60))
    args_to = float(request.args.get('to', 0))

    t0 = time.time()
    data, timestamps = current_app.eeg_buffer.get_timerange(
        t0 + args_from,
        t0 + args_to,
    )
    timestamps_relative = [t - t0 for t in timestamps]

    result = {
        'data': data.tolist(),
        'timestamps': timestamps_relative,
    }
    return json.dumps(result), 200, {'content-type': 'application/json'}
