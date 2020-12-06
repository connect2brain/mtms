import logging

import win32com.client

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s',)

class MtmsConnection():
    def __init__(self, app_name=None, app_location=None, app_filename=None, vi_name=None):
        path_to_vi = app_location + '\\' + app_filename + '\\' + vi_name

        logging.info('Using ActiveX to connect to application ' + app_name)
        self.mtms_app = win32com.client.Dispatch(app_name + '.Application')
        self.vi = self.mtms_app.getvireference(path_to_vi)

    def set_value(self, control_name=None, value=None):
        logging.info('Setting value {} for control {}'.format(value, control_name))
        self.vi.SetControlValue(control_name, value)
