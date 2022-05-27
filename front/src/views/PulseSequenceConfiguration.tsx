import React, { useEffect, useMemo, useState } from 'react'
import { ChannelInfoWithEnabled, Experiment, PulseSequence } from '../types/pulseSequence'
import styled from 'styled-components'
import useStore from '../providers/state'

const Channel = ({ channelIndex, enabled, voltage }: ChannelInfoWithEnabled) => {
  const { channels, setChannels } = useStore((state) => state)

  const handleOnChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const target = event.target.name
    const value = event.target.type === 'checkbox' ? event.target.checked : event.target.value
    const newChannels = [...channels]
    const oldChannel = {
      channelIndex,
      enabled,
      voltage
    }
    newChannels[channelIndex - 1] = {
      ...oldChannel,
      [target]: value
    }
    setChannels(newChannels)
  }

  return (
    <ChannelContainer enabled={enabled}>
      <input name='enabled' type='checkbox' onChange={handleOnChange} />
      Channel {channelIndex} {' '}
      <label>Voltage: </label>
      <input name='voltage' type='number' disabled={!enabled} value={voltage} onChange={handleOnChange}/>
    </ChannelContainer>
  )
}

const ChannelContainer = styled.div<{
  enabled: boolean
}>`
  color: ${(p) => (p.enabled ? p.theme.colors.primary : p.theme.colors.grey)};
`
/** TODO: button to add additional channels */
const PulseSequenceConfiguration = () => {
  const { channels, setChannels } = useStore((state) => state)

  const [description, setDescription] = useState<string>('')

  return (
    <div>
      <form>
        <ChannelsContainer>
          {channels.map((channel) => (
            <Channel
              key={`channel-${channel.channelIndex}`}
              {...channel}
            />
          ))}
        </ChannelsContainer>
      </form>
    </div>
  )
}

const ChannelsContainer = styled.div``

export default PulseSequenceConfiguration
