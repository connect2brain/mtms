import ROSLIB from 'roslib'
import { ROS_URL } from '../utils/constants'
import { PositionMessage, StateMessage } from '../types/target'
import { EegBatchMessage, EegDatapointMessage, EegTriggerMessage } from '../types/eeg'

export const ros = new ROSLIB.Ros({
  url: ROS_URL,
})

ros.on('connection', () => {
  console.log('ROS connected to', ROS_URL)
})

ros.on('error', (error) => {
  console.log('Error connecting ROS to', ROS_URL, ',error:', error)
})

ros.on('close', () => {
  console.log('ROS closed ws connection')
})

export const coilPositionSubscriber = new ROSLIB.Topic<PositionMessage>({
  ros: ros,
  name: '/neuronavigation/focus',
  messageType: 'neuronavigation_interfaces/PoseUsingEulerAngles',
})

/* Set up get maximum intensity service.*/
const getMaximumIntensityService = new ROSLIB.Service({
  ros: ros,
  name: '/targeting/get_maximum_intensity',
  serviceType: 'targeting_interfaces/GetMaximumIntensity',
})

export const getMaximumIntensity =
    (x: number, y: number, angle: number, callback: (maximum_intensity: number) => void) => {
  const request = new ROSLIB.ServiceRequest({
    target: {
      displacement_x: x,
      displacement_y: y,
      rotation_angle: angle
    }
  }) as any

  getMaximumIntensityService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to get maximum intensity')
      } else {
        callback(response.maximum_intensity)
      }
    },
    (error) => {
      console.log('ERROR: Failed to get maximum intensity, error:')
      console.log(error)
    },
  )
}

/* Set up toggle_navigation service.*/
const toggleNavigationService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/toggle_navigation',
  serviceType: 'ui_interfaces/ToggleNavigation',
})

const clearStateService = new ROSLIB.Service({
  ros: ros,
  name: '/planner/clear_state',
  serviceType: 'ui_interfaces/ClearState',
})

/* Set up listener for coil at target. */
const coilAtTargetListener = new ROSLIB.Topic({
  ros: ros,
  name: '/neuronavigation/coil_at_target',
  messageType: 'std_msgs/Bool',
})
//coilAtTargetListener.subscribe(updateCoilAtTarget);

/* Set up listener for planner state. */
export const plannerStateSubscriber = new ROSLIB.Topic<StateMessage>({
  ros: ros,
  name: '/planner/state',
  messageType: 'ui_interfaces/PlannerState',
})

/* Set up listener for eeg data. */
export const eegDataSubscriber = new ROSLIB.Topic<EegBatchMessage>({
  ros: ros,
  name: '/eeg/batch_data',
  messageType: 'ui_interfaces/EegBatch',
})

/* Set up listener for node messages. */
export const nodeMessageSubscriber = new ROSLIB.Topic({
  ros: ros,
  name: '/node/message',
  messageType: 'std_msgs/String',
})

/* Set up listener for planner state. */
export const triggerSubscriber = new ROSLIB.Topic<EegTriggerMessage>({
  ros: ros,
  name: '/eeg/trigger_received',
  messageType: 'ui_interfaces/Trigger',
})

/* Set up start_session service */
export const startSessionService = new ROSLIB.Service({
  ros: ros,
  name: '/stimulation/start_session',
  serviceType: 'ui_interfaces/ToggleNavigation',
})

export const clearRosState = () => {
  const request = new ROSLIB.ServiceRequest({})
  console.log('clearing ros state')
  clearStateService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to clear state')
      }
    },
    (error) => {
      console.log('ERROR: Failed to clear state, error:')
      console.error(error)
    },
  )
}

/* Set up count valid trials service. */
const countValidTrialsService = new ROSLIB.Service({
  ros: ros,
  name: '/experiment/count_valid_trials',
  serviceType: 'experiment_interfaces/CountValidTrials',
})

export const countValidTrials =
    (trials: any, callback: (numOfValidTrials: number) => void) => {
  const request = new ROSLIB.ServiceRequest({
    trials: trials
  }) as any

  countValidTrialsService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to count valid trials: success field was false.')
      } else {
        callback(response.num_of_valid_trials)
      }
    },
    (error) => {
      console.log('ERROR: Failed to count valid trials, error:')
      console.log(error)
    },
  )
}

/* Set up perform experiment service. */
const performExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/experiment/perform_service',
  serviceType: 'experiment_interfaces/PerformExperimentService',
})

export const performExperiment =
    (experiment: any, callback: (trialResults: any, success: boolean) => void) => {
  const request = new ROSLIB.ServiceRequest(experiment) as any

  performExperimentService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to perform experiment: success field was false.')
        callback(response.trial_results, false)
      } else {
        callback(response.trial_results, true)
      }
    },
    (error) => {
      console.log('ERROR: Failed to perform experiment, error:')
      console.log(error)
    },
  )
}

/* Set up pause experiment service. */
const pauseExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/experiment/pause',
  serviceType: 'experiment_interfaces/PauseExperiment',
})

export const pauseExperiment = (callback: () => void) => {
  const request = new ROSLIB.ServiceRequest({}) as any

  pauseExperimentService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to pause experiment: success field was false.')
      } else {
        callback()
      }
    },
    (error) => {
      console.log('ERROR: Failed to pause experiment, error:')
      console.log(error)
    },
  )
}

/* Set up resume experiment service. */
const resumeExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/experiment/resume',
  serviceType: 'experiment_interfaces/ResumeExperiment',
})

export const resumeExperiment = (callback: () => void) => {
  const request = new ROSLIB.ServiceRequest({}) as any

  resumeExperimentService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to resume experiment: success field was false.')
      } else {
        callback()
      }
    },
    (error) => {
      console.log('ERROR: Failed to resume experiment, error:')
      console.log(error)
    },
  )
}

/* Set up list projects service. */
const listProjectsService = new ROSLIB.Service({
  ros: ros,
  name: '/projects/list',
  serviceType: 'project_interfaces/ListProjects',
})

export const listProjects = (callback: (projects: string[]) => void) => {
  const request = new ROSLIB.ServiceRequest({}) as any

  listProjectsService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to list projects: success field was false.')
      } else {
        callback(response.projects)
      }
    },
    (error) => {
      console.log('ERROR: Failed to list projects, error:')
      console.log(error)
    },
  )
}

/* Set up set active project service. */
const setActiveProjectService = new ROSLIB.Service({
  ros: ros,
  name: '/projects/active/set',
  serviceType: 'project_interfaces/SetActiveProject',
})

export const setActiveProject = (project: string, callback: () => void) => {
  const request = new ROSLIB.ServiceRequest({
    project: project
  }) as any

  setActiveProjectService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to set active project: success field was false.')
      } else {
        callback()
      }
    },
    (error) => {
      console.log('ERROR: Failed to set active project, error:')
      console.log(error)
    },
  )
}
