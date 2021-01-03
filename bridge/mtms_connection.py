import os
import logging

import dotenv
import win32com.client

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

class MtmsConnection():
    def __init__(self):
        dotenv.load_dotenv()  # Load configuration from env vars and .env -file

        app_name = os.getenv('MTMS_APP_NAME', "MTMSActiveXServer")
        app_location = os.getenv('MTMS_APP_LOCATION')
        app_filename = os.getenv('MTMS_APP_FILENAME')
        vi_name = os.getenv('MTMS_VI_NAME', "mTMS ActiveX Server.vi")

        path_to_vi = app_location + '\\' + app_filename + '\\' + vi_name

        logging.info('Using ActiveX to connect to application ' + app_name)
        self._mtms_app = win32com.client.Dispatch(app_name + '.Application')
        self._vi = self._mtms_app.getvireference(path_to_vi)

    def set_value(self, control_name=None, value=None):
        logging.info('Setting value {} for control {}'.format(value, control_name))
        self._vi.SetControlValue(control_name, value)

    def get_value(self, control_name=None):
        value = self._vi.GetControlValue(control_name)
        logging.info('Getting value {} for control {}'.format(value, control_name))
        return value
