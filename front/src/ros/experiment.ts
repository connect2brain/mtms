import ROSLIB from '@foxglove/roslibjs'
import { ros } from './ros'

/* Get maximum intensity service */
const getMaximumIntensityService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/targeting/get_maximum_intensity',
  serviceType: 'targeting_interfaces/GetMaximumIntensity',
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
    algorithm: algorithm,
  }) as any

  getMaximumIntensityService.callService(
    request,
    (response: any) => {
      if (!response.success) {
        console.log('ERROR: Failed to get maximum intensity')
      } else {
        callback(response.maximum_intensity)
      }
    },
    (error: any) => {
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
    (response: any) => {
      if (!response.success) {
        console.log('ERROR: Failed to count valid trials: success field was false.')
      } else {
        callback(response.num_of_valid_trials)
      }
    },
    (error: any) => {
      console.log('ERROR: Failed to count valid trials, error:')
      console.log(error)
    }
  )
}

/* Perform experiment service */
const performExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/experiment/perform',
  serviceType: 'experiment_interfaces/PerformExperiment',
})

/* Experiment feedback topic */
const experimentFeedbackTopic = new ROSLIB.Topic({
  ros: ros,
  name: '/mtms/experiment/feedback',
  messageType: 'experiment_interfaces/ExperimentFeedback',
})

export const performExperiment = (
  experiment: any,
  done_callback: (trialResults: any, success: boolean) => void,
  feedback_callback: (response: any) => void
) => {
  const request = new ROSLIB.ServiceRequest({
    experiment: experiment,
  })

  let previousExperimentState: number | null = null
  const onFeedback = (feedback: any) => {
    const currentExperimentState = feedback?.state
    if (typeof currentExperimentState === 'number') {
      if (currentExperimentState === 0) {
        experimentFeedbackTopic.unsubscribe(onFeedback)
        done_callback([], previousExperimentState !== 3)
        return
      }
      previousExperimentState = currentExperimentState
    }
    feedback_callback(feedback)
  }
  experimentFeedbackTopic.subscribe(onFeedback)

  performExperimentService.callService(
    request,
    (response: any) => {
      if (!response.success) {
        experimentFeedbackTopic.unsubscribe(onFeedback)
        console.log('ERROR: Failed to perform experiment: success field was false.')
        done_callback([], false)
      }
    },
    (error: any) => {
      experimentFeedbackTopic.unsubscribe(onFeedback)
      console.log('ERROR: Failed to perform experiment, error:')
      console.log(error)
      done_callback([], false)
    }
  )
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
    (response: any) => {
      if (!response.success) {
        console.log('ERROR: Failed to visualize targets: success field was false.')
      } else {
        callback()
      }
    },
    (error: any) => {
      console.log('ERROR: Failed to visualize targets, error:')
      console.log(error)
    }
  )
}

/* Pause experiment service */
const pauseExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/experiment/pause',
  serviceType: 'std_srvs/Trigger',
})

export const pauseExperiment = (callback: () => void) => {
  const request = new ROSLIB.ServiceRequest({}) as any

  pauseExperimentService.callService(
    request,
    (response: any) => {
      if (!response.success) {
        console.log('ERROR: Failed to pause experiment: success field was false.')
      } else {
        callback()
      }
    },
    (error: any) => {
      console.log('ERROR: Failed to pause experiment, error:')
      console.log(error)
    }
  )
}

/* Resume experiment service */
const resumeExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/experiment/resume',
  serviceType: 'std_srvs/Trigger',
})

export const resumeExperiment = (callback: () => void) => {
  const request = new ROSLIB.ServiceRequest({}) as any

  resumeExperimentService.callService(
    request,
    (response: any) => {
      if (!response.success) {
        console.log('ERROR: Failed to resume experiment: success field was false.')
      } else {
        callback()
      }
    },
    (error: any) => {
      console.log('ERROR: Failed to resume experiment, error:')
      console.log(error)
    }
  )
}

/* Cancel experiment service */
const cancelExperimentService = new ROSLIB.Service({
  ros: ros,
  name: '/mtms/experiment/cancel',
  serviceType: 'std_srvs/Trigger',
})

export const cancelExperiment = (callback: () => void) => {
  const request = new ROSLIB.ServiceRequest({}) as any

  cancelExperimentService.callService(
    request,
    (response: any) => {
      if (!response.success) {
        console.log('ERROR: Failed to cancel experiment: success field was false.')
      } else {
        callback()
      }
    },
    (error: any) => {
      console.log('ERROR: Failed to cancel experiment, error:')
      console.log(error)
    }
  )
}
