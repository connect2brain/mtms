import React from 'react'

import { startDevice, stopDevice } from 'ros/services/device'
import { DeviceState, DeviceStateMessage } from 'types/mtmsDevice'
import { StyledButton } from 'styles/General'

type Props = {
  deviceState: DeviceStateMessage
}

export const DeviceControl = ({ deviceState }: Props) => {
  const deviceText = () => {
    if (deviceState.value === DeviceState.NOT_OPERATIONAL) return 'Start'
    if (deviceState.value === DeviceState.STARTUP) return 'Starting'
    if (deviceState.value === DeviceState.OPERATIONAL) return 'Stop'
    if (deviceState.value === DeviceState.SHUTDOWN) return 'Stopping'
    return '???'
  }

  const toggleDevice = () => {
    if (deviceState.value === DeviceState.NOT_OPERATIONAL) {
      startDevice()
    } else if (deviceState.value === DeviceState.OPERATIONAL) {
      stopDevice()
    }
  }

  return (
    <>
      <StyledButton
        onClick={toggleDevice}
        disabled={deviceState.value === DeviceState.STARTUP || deviceState.value === DeviceState.SHUTDOWN}
      >
        {deviceText()} device
      </StyledButton>
    </>
)
}
