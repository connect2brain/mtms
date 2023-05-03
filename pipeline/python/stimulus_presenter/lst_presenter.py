from .base.events import TriggerOut
from psychopy import visual, core
import numpy as np
import serial

from enum import Enum


class ExecutionCondition(Enum):
    TIMED = 0
    WAIT_FOR_TRIGGER = 1
    IMMEDIATE = 2


# reminder to set permissions to use parallel port from the shell
# sudo chmod 666 /dev/ttyUSB0
# Initialize USB/parallel port interface

# port_address = '/dev/ttyUSB0' # or None if no parallel port is present
# if port_address:
#     port = parallel.ParallelPort(port_address)
#     port.setData(0)
# else:
#     port = None


class PresenterMode(Enum):
    BASELINE = 1
    BREAK = 2
    BLOCK_INIT = 3
    BLOCK = 4


class PresenterModeUpdate(Enum):
    NO_UPDATE = 1
    SKIP_BASELINE = 2
    INIT_BREAK = 3
    INIT_BLOCK = 4
    START_BLOCK = 5


class PipelineStage:
    def __init__(self, skip_port = False):
        self.mywin = None
        self.stimbar = None
        self.text_stim = None
        self.circle = None

        self.nsteps = 10        # Number of bar height levels
        self.steps = None       # Heights of each level, initialize stimbar first
        self.cur_step = 4       # Level to start from

        self.nblocks = 11           # 11. Number of training blocks to run + baseline
        
        self.init_block_duration = 5
        self.block_duration = 120    # 120 + 5 ignored seconds in the beginning of the block. Duration of one block in seconds
        self.break_duration = 55    # Duration of break in seconds + init_block_duration 
        self.quit_duration = 20

        # current mode
        self.mode = PresenterMode.BASELINE

        # internal timer (in seconds)
        # define a stop time for current mode
        self.time_end = 0
        # list of (time, action) to be executed during current mode
        # for example we want to update countdown text every second
        self.timer_actions = None

        self.skip_port = skip_port
        


    def set_testing_params(self, nblocks = 3, break_duration = 5, init_block_duration = 3, block_duration = 5, quit_duration = 3):
        self.nblocks = nblocks
        self.init_block_duration = init_block_duration
        self.break_duration = break_duration
        self.block_duration = block_duration
        self.quit_duration = quit_duration


    def init_experiment(self):
        self.mywin = visual.Window(
            size=[900, 600],
            monitor="testMonitor",
            units="pix",
            fullscr=True,
            color=[0.75, 0.75, 0.75],
            )
        self.block = 1               # Current block running, 1=baseline
        if not self.skip_port:
            self.port_address = '/dev/ttyUSB1' # or None if no parallel port is present
            self.port = serial.Serial(self.port_address, baudrate=115200) #change baudrate
            # 
        if not self.port.is_open:
            self.port.open()

        self.mywin.flip()
        core.wait(5.)


        
        # Run baseline block as first block:
        self.init_baseline()
        return []


    def init_baseline(self):
        # Window during baseline. View a black circle
        self.circle = visual.Circle(
            win = self.mywin,
            units = "pix",
            radius = 0.015 * self.mywin.size[0],
            edges = 30,
            fillColor = [-1, -1, -1],
            lineColor = [-1, -1, -1],
            autoDraw=False,
            pos = (0, 0),
            )

        self.circle.draw()
        self.mywin.flip()
        return []


    def clear_baseline_graphics(self):
        # clear circle
        self.circle.fillColor = None  # clear the fill color of the circle
        self.circle.lineColor = None  # clear the line color of the circle
        self.circle.radius = 0  # set the radius of the circle to 0 to make it disappear
        self.mywin.flip()
        return


    def init_break(self, execution_time):
        print(f"Break invoked at {execution_time}")
        if self.block == self.nblocks:
            self.end_experiment()
        else:
            # View a short message during the break.
            break_text = 'Break!\n' + str(self.block) + ' blocks done.\n' + str(self.nblocks - self.block) + ' blocks to go.'

            self.text_stim = visual.TextStim(self.mywin,
                text = break_text,
                autoDraw=False,
                color = [-1, -1, -1],
                height = 60
                )

        self.text_stim.draw()
        self.mywin.flip()
        self.trigger(8)
        # Press any button to continue with experiment
        self.mode = PresenterMode.BREAK
        self.set_timer(execution_time, self.break_duration)
        return []


    def init_block(self, execution_time):
        self.text_stim = visual.TextStim(self.mywin, text = '',
                autoDraw=False,
                color = [-1, -1, -1],
                height = 60
                )
        self.mode = PresenterMode.BLOCK_INIT
        # update countdown text every second
        step = 1 #seconds
        def update_countdown_text(max_t, i):
            self.text_stim.setText('Next block starts in ' + str(max_t - i) + ' seconds.\n')
            self.text_stim.draw()
            self.mywin.flip()
        countdown_action = lambda i: update_countdown_text(self.init_block_duration, i)
        self.set_timer(execution_time, self.init_block_duration, step, countdown_action)
        return []


    def start_block(self, execution_time):
        # clear text
        self.text_stim.setText('')
        self.text_stim.draw()

        self.stimbar = visual.Rect(
            win=self.mywin,
            units="pix",
            size=(0.1 * self.mywin.size[0], 0.8 * self.mywin.size[1]),
            fillColor=[-1, -1, -1],
            lineColor=[-1, -1, -1],
            autoDraw=False,
            pos=(0, 0),
        )

        self.steps = np.linspace(self.stimbar.verticesPix[3, 1], self.stimbar.verticesPix[0, 1], self.nsteps)

        # Update the bar height and screen, always start new block from same level?
        self.cur_step = 4
        self.update_height(self.cur_step)

        self.stimbar.draw()
        self.mywin.flip()

        self.mode = PresenterMode.BLOCK
        self.block = self.block + 1
        self.set_timer(execution_time, self.block_duration)
        return []


    def end_experiment(self):
        message = visual.TextStim(self.mywin,
                text='We are done!\nWindow closes in ' + str(self.quit_duration)+ ' seconds.',
                autoDraw=False,
                color = [-1, -1, -1],
                height = 60
                )

        message.draw()
        self.block = 1               # Reset current block
        self.mywin.flip()
        #event.waitKeys()
        self.port.close()
        core.wait(self.quit_duration)
        self.mywin.close()

        #quit()
        return []


    def update_height(self, step):
        self.stimbar.verticesPix[0, 1] = self.steps[step]
        self.stimbar.verticesPix[1, 1] = self.steps[step]

        self.stimbar.draw()
        self.mywin.flip()
        return []


    def trigger(self, x):
        # pass
        self.port.write(bytearray([x]))
        core.wait(0.005)
        self.port.write(bytearray([0]))
        self.port.flushOutput()
        # Use trigger(x) whenever you need to send a trigger code


    def set_timer(self, cur_time, duration, step=None, action=None):
        # TODO: check time format and convert duration if necessary
        # we assume it is in seconds
        self.time_end = cur_time + duration
        if step is not None and action is not None:
            steps = duration // step
            self.timer_actions = [(cur_time + t, lambda i = t: action(i)) for t in range(steps)]


    def do_timer_action(self, cur_time):
        if self.timer_actions is None:
            return
        if len(self.timer_actions) == 0:
            return
        # get next action
        t, action = self.timer_actions[0]
        # if it is time to do the action
        if cur_time >= t:
            action()
            self.timer_actions.pop(0)


    # check timer and decide what to do next
    def get_ui_update_command(self, state, cur_time):
        # if baseline is over
        if state == 5:
            return PresenterModeUpdate.INIT_BREAK

        # no updates in baseline mode
        if self.mode == PresenterMode.BASELINE:
            return PresenterModeUpdate.SKIP_BASELINE

        # if timer is still running
        if cur_time < self.time_end:
            return PresenterModeUpdate.NO_UPDATE

        # transition map
        transition_map = {PresenterMode.BREAK: PresenterModeUpdate.INIT_BLOCK,
                          PresenterMode.BLOCK_INIT: PresenterModeUpdate.START_BLOCK,
                          PresenterMode.BLOCK: PresenterModeUpdate.INIT_BREAK}
        # clean actions list
        self.timer_actions = None
        # return next mode or NO_UPDATE if no update is needed
        return transition_map.get(self.mode, PresenterModeUpdate.NO_UPDATE)


    # return True if data should be analyzed
    # return False if no data should be analyzed (e.g. in baseline mode or break)
    def update_ui(self, update_command, execution_time):
        self.do_timer_action(execution_time)

        if update_command == PresenterModeUpdate.SKIP_BASELINE:
            return False

        if update_command == PresenterModeUpdate.INIT_BREAK:
            if (self.mode == PresenterMode.BASELINE):
                self.clear_baseline_graphics()
            self.init_break(execution_time)
            return False

        if update_command == PresenterModeUpdate.INIT_BLOCK:
            self.init_block(execution_time)
            return False

        if update_command == PresenterModeUpdate.START_BLOCK:
           self.start_block(execution_time)
           return False

        return update_command == PresenterModeUpdate.NO_UPDATE and self.mode not in (PresenterMode.BREAK, PresenterMode.BLOCK_INIT)


    def update_step(self, state):
        # Decrease bar height if height is not at minimum
        if state == 2 and self.cur_step != 0:
            self.cur_step = max(0, self.cur_step - 1)
            return 2
        # Increase bar height if height is not at maximum
        if state == 3 and self.cur_step != self.nsteps - 1:
            self.cur_step = min(self.nsteps - 1, self.cur_step + 1)
            return 3
        # Do nothing
        return 1


    def duration_by_state(self, state):
        if state == 2:
            return 7000
        if state == 3:
            return 11000
        return 3000


    def data_received(self, execution_time, state):
        # Check code or timer to update mode
        update_command = self.get_ui_update_command(state, execution_time)

        # update ui if necessary
        should_analyze = self.update_ui(update_command, execution_time)
        # if ui was updated or break/baseline, return empty list
        if not should_analyze:
            return []

        print(f"Stimulus received: {state} at {execution_time}")
        state = self.update_step(state)
        event_info = {
            "id": state,
            "execution_condition": ExecutionCondition.IMMEDIATE.value,
            "execution_time": 1,
            "decision_time": execution_time
        }
        event = TriggerOut(port=1, duration_us=self.duration_by_state(state), event_info=event_info)

        self.update_height(self.cur_step)
        if not self.skip_port:
            self.trigger(state)
        print('Output state is: ' + str(state))
        return [event]
