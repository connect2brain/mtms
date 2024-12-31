import wx

import rclpy
from rclpy.node import Node
from std_msgs.msg import Bool

class PedalBridge(Node):
    def __init__(self):
        super().__init__("pedal_bridge")

        self._pedal_pressed_subscription = self.create_subscription(Bool, "/pedal/left_button/pressed", self.pedal_pressed, 10)
        self._pedal_callbacks = []

    def pedal_pressed(self, msg):
        state = msg.data

        for pedal_callback in self._pedal_callbacks:
            callback = pedal_callback['callback']

            # wx.CallAfter wrapping is needed to make callbacks that update WxPython UI not crash with a segfault,
            # as the ROS node is run in a different thread than the neuronavigation. (Search for WxPython and thread
            # safety for more information.)
            #
            wx.CallAfter(callback, state)

        if not state:
            self._pedal_callbacks = [callback for callback in self._pedal_callbacks if not callback['remove_when_released']]

    def add_pedal_callback(self, name, callback, remove_when_released):
        self._pedal_callbacks.append({
            'name': name,
            'callback': callback,
            'remove_when_released': remove_when_released,
        })

    def remove_pedal_callback(self, name):
        self._pedal_callbacks = [callback for callback in self._pedal_callbacks if callback['name'] != name]
