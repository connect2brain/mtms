import { ChannelState as ChannelStateType } from 'types/fpga'
import React from 'react'

export const ChannelState = (state: ChannelStateType) => {
  return (
    <div>
      <tr>
        <td>{state.channelIndex}</td>
        <td>{state.voltage}</td>
        <td>{state.temperature}</td>
        <td>{state.pulseCount}</td>
        <td>{state.error}</td>
      </tr>
    </div>
  )
}
