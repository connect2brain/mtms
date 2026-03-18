import ROSLIB from '@foxglove/roslibjs'
import { ros } from './ros'

/* Get maximum intensity service */
const getMaximumIntensityService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/targeting/get_maximum_intensity',
  serviceType: 'targeting_services/GetMaximumIntensity',
})

export const getMaximumIntensity = (
  x: number,
  y: number,
  angle: number,
  algorithm: number,
  callback: (maximum_intensity: number) => void
) => {
  const request = new ROSLIB.ServiceRequest({
    displacement_x: x,
    displacement_y: y,
    rotation_angle: angle,
    algorithm: {
      value: algorithm,
    },
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
    }
  )
}

/* Count valid trials service */
const countValidTrialsService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/experiment/count_valid_trials',
  serviceType: 'experiment_interfaces/CountValidTrials',
})

export const countValidTrials = (trials: any, callback: (numOfValidTrials: number) => void) => {
  const request = new ROSLIB.ServiceRequest({
    trials: trials,
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
    }
  )
}

/* Perform experiment action.
   Foxglove roslibjs uses the RobotWebTools ActionClient/Goal API. */
const performExperimentActionClient: any = new (ROSLIB as any).ActionClient({
  ros: ros,
  name: '/mtms/experiment/perform',
  actionName: 'experiment_interfaces/PerformExperimentAction',
})

export const performExperiment = (
  experiment: any,
  done_callback: (trialResults: any, success: boolean) => void,
  feedback_callback: (response: any) => void
) => {
  const goal: any = new (ROSLIB as any).Goal({
    actionClient: performExperimentActionClient,
    goalMessage: {
      experiment: experiment,
    },
  })


  console.log('Sending goal to perform experiment')

  goal.on('result', (response: any) => {
    console.log('Result received from perform experiment')
    if (!response.success) {
      console.log('ERROR: Failed to perform experiment: success field was false.')
      done_callback(response.trial_results, false)
    } else {
      done_callback(response.trial_results, true)
    }
  })

  goal.on('feedback', (feedback: any) => {
    feedback_callback(feedback)
  })

  goal.send()

  console.log('Goal sent to perform experiment')
}

/* Visualize targets service */
const visualizeTargetsService = new ROSLIB.Service({
  ros: ros,
  name: '/neuronavigation/visualize/targets',
  serviceType: 'neuronavigation_interfaces/VisualizeTargets',
})

export const visualizeTargets = (targets: any, is_ordered: boolean, callback: () => void) => {
  const request = new ROSLIB.ServiceRequest({}) as any

  request.targets = targets
  request.is_ordered = is_ordered

  visualizeTargetsService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to visualize targets: success field was false.')
      } else {
        callback()
      }
    },
    (error) => {
      console.log('ERROR: Failed to visualize targets, error:')
      console.log(error)
    }
  )
}

/* Pause experiment service */
const pauseExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/experiment/pause',
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
    }
  )
}

/* Resume experiment service */
const resumeExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/experiment/resume',
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
    }
  )
}

/* Cancel experiment service */
const cancelExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/experiment/cancel',
  serviceType: 'experiment_interfaces/CancelExperiment',
})

export const cancelExperiment = (callback: () => void) => {
  const request = new ROSLIB.ServiceRequest({}) as any

  cancelExperimentService.callService(
    request,
    (response) => {
      if (!response.success) {
        console.log('ERROR: Failed to cancel experiment: success field was false.')
      } else {
        callback()
      }
    },
    (error) => {
      console.log('ERROR: Failed to cancel experiment, error:')
      console.log(error)
    }
  )
}
