import { ChannelState as ChannelStateType } from 'types/mtmsDevice'
import React from 'react'
import { getTrueKeys } from '../utils'

export const ChannelState = (state: ChannelStateType) => {
  const getListValue = (object: any) => {
    const keys: string[] = getTrueKeys(object)

    if (keys.length > 0) {
      return keys.map((key) => {
        return <span key={key}>{key}</span>
      })
    } else {
      /* No errors, do not display anything. */
      return <span></span>
    }
  }

  return (
      <tr>
        <td>{state.channel_index}</td>
        <td>{state.voltage}</td>
        {/*<td>{state.temperature}</td>*/}
        <td>{state.pulse_count}</td>
        <td>{getListValue(state.channel_error)}</td>
      </tr>
  )
}
