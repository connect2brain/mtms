import React, { useEffect, useMemo, useState } from 'react'
import { ChannelInfoWithEnabled, Experiment, PulseSequence } from '../types/pulseSequence'
import styled from 'styled-components'
import useStore from '../providers/state'

const Channel = (props: ChannelInfoWithEnabled) => {
  const { channelIndex, enabled, voltage } = props
  const { channels, setChannels } = useStore((state) => state)

  const handleOnChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const target = event.target.name
    const value = event.target.type === 'checkbox' ? event.target.checked : event.target.value
    const newChannels = [...channels]
    newChannels[channelIndex - 1] = {
      ...props,
      [target]: value,
    }
    setChannels(newChannels)
  }

  return (
    <ChannelContainer enabled={enabled}>
      <input name='enabled' type='checkbox' onChange={handleOnChange} />
      Channel {channelIndex} <label htmlFor='voltage'>Voltage: </label>
      <input name='voltage' type='number' disabled={!enabled} value={voltage} onChange={handleOnChange} />
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

  const startSequence = (event: any) => {
    const sequenceData = {}

    console.log('starting sequence:', sequenceData)
  }

  return (
    <div>
      <label htmlFor='iti'>Inter-train interval (iti) {' '}</label>
      <input name='iti' type='number' value={500}/>
      <br/>
      <label htmlFor='ibi'>Inter-burst interval (ibi) {' '}</label>
      <input name='ibi' type='number' value={500}/>
      <br/>
      <label htmlFor='nofPulsesInBursts'>Number of pulses in bursts {' '}</label>
      <input name='nofPulsesInBursts' type='number' value={3}/>
      <br/>
      <label htmlFor='nofBurstsInTrains'>Number of bursts in trains {' '}</label>
      <input name='nofBurstsInTrains' type='number' value={3}/>
      <br/>
      <label htmlFor='nofTrains'>Number of trains {' '}</label>
      <input name='nofTrains' type='number' value={1}/>
      <br/>
      <br/>

      <h2>Channels</h2>
      <ChannelsContainer>
        {channels.map((channel) => (
          <Channel key={`channel-${channel.channelIndex}`} {...channel} />
        ))}
      </ChannelsContainer>

      <br/>
      <button onClick={startSequence}>Start sequence</button>
    </div>
  )
}

const ChannelsContainer = styled.div``

export default PulseSequenceConfiguration
