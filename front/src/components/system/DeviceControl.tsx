import React, { useContext } from 'react'

import { startDevice, stopDevice } from 'ros/device'
import { StyledButton } from 'styles/General'

import { SystemContext, DeviceState } from 'providers/SystemProvider'

export const DeviceControl = () => {
  const { systemState } = useContext(SystemContext)

  const deviceState = systemState?.device_state

  const deviceText = () => {
    switch (deviceState?.value) {
      case DeviceState.NOT_OPERATIONAL:
        return 'Start device'

      case DeviceState.STARTUP:
        return 'Starting...'

      case DeviceState.OPERATIONAL:
        return 'Stop device'

      case DeviceState.SHUTDOWN:
        return 'Stopping...'
    }
    return 'Start device'
  }

  const toggleDevice = () => {
    if (deviceState?.value === DeviceState.NOT_OPERATIONAL) {
      startDevice()
    } else if (deviceState?.value === DeviceState.OPERATIONAL) {
      stopDevice()
    }
  }
  return (
    <StyledButton
      onClick={toggleDevice}
      disabled={
        deviceState?.value === DeviceState.STARTUP ||
        deviceState?.value === DeviceState.SHUTDOWN ||
        deviceState?.value === undefined
      }
    >
      {deviceText()}
    </StyledButton>
  )
}
