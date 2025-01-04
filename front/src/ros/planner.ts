import ROSLIB from 'roslib'
import { ros } from './ros'

import { StateMessage } from '../types/target'

/* Listener for planner state */
export const plannerStateSubscriber = new ROSLIB.Topic<StateMessage>({
  ros: ros,
  name: '/planner/state',
  messageType: 'ui_interfaces/PlannerState',
})

/* Toggle navigation service */
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
    }
  )
}
