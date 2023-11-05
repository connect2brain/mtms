import React, { useContext } from 'react'

import { startDevice, stopDevice } from 'ros/services/device'
import { StyledButton } from 'styles/General'

import { SystemContext, DeviceState } from 'providers/SystemProvider'

export const DeviceControl = () => {
  const { systemState } = useContext(SystemContext)

  const deviceState = systemState?.device_state

  const deviceText = () => {
    if (deviceState?.value === DeviceState.NOT_OPERATIONAL) return 'Start'
    if (deviceState?.value === DeviceState.STARTUP) return 'Starting'
    if (deviceState?.value === DeviceState.OPERATIONAL) return 'Stop'
    if (deviceState?.value === DeviceState.SHUTDOWN) return 'Stopping'
    return '???'
  }

  const toggleDevice = () => {
    if (deviceState?.value === DeviceState.NOT_OPERATIONAL) {
      startDevice()
    } else if (deviceState?.value === DeviceState.OPERATIONAL) {
      stopDevice()
    }
  }

  return (
    <>
      <StyledButton
        onClick={toggleDevice}
        disabled={deviceState?.value === DeviceState.STARTUP || deviceState?.value === DeviceState.SHUTDOWN}
      >
        {deviceText()} device
      </StyledButton>
    </>
  )
}
