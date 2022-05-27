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
  const {
    channels,
    setChannels,
    iti,
    ibi,
    nofBurstsInTrains,
    nofTrains,
    isis,
    setIti,
    setIbi,
    setNofBurstsInTrains,
    setNofTrains,
    setIsis,
    nofPulsesInBursts,
    setNofPulsesInBursts,
  } = useStore((state) => state)

  const startSequence = (event: any) => {
    const sequenceData = {}

    console.log('starting sequence:', sequenceData)
  }

  const isiChangeHandler = (value: string, index: number) => {
    const newIsis = [...isis]
    newIsis[index] = Number(value)
    setIsis(newIsis)
  }

  const nofPulsesInBurstChangeHandler = (value: number) => {
    const last = isis.length > 0 ? isis[isis.length - 1] : 500

    const nofElementsToAdd = value - isis.length

    let newIsis: number[]
    if (nofElementsToAdd > 0) {
      const elementsToAdd = Array(nofElementsToAdd).fill(last)
      newIsis = isis.concat(...elementsToAdd)
    } else {
      newIsis = isis.slice(0, value)
    }

    setNofPulsesInBursts(value)
    setIsis(newIsis)
  }

  return (
    <div>
      <label htmlFor='iti'>Inter-train interval (iti) </label>
      <input name='iti' type='number' value={iti} min={0} onChange={(event) => setIti(Number(event.target.value))} />
      <br />
      <label htmlFor='ibi'>Inter-burst interval (ibi) </label>
      <input name='ibi' type='number' value={ibi} min={0} onChange={(event) => setIbi(Number(event.target.value))} />
      <br />

      <label htmlFor='nofPulsesInBursts'>Number of pulses in bursts </label>
      <input
        name='nofPulsesInBursts'
        type='number'
        min={0}
        value={nofPulsesInBursts}
        onChange={(event) => nofPulsesInBurstChangeHandler(Number(event.target.value))}
      />
      <br />
      <label htmlFor='nofBurstsInTrains'>Number of bursts in trains </label>
      <input
        name='nofBurstsInTrains'
        type='number'
        min={0}
        value={nofBurstsInTrains}
        onChange={(event) => setNofBurstsInTrains(Number(event.target.value))}
      />
      <br />
      <br />

      {isis.map((isi, index) => {
        return (
          <div key={`isi-${index}`}>
            <label htmlFor={`isi-${index}`}>Interstimulus interval (isi) {index + 1} </label>
            <input
              name={`isi-${index}`}
              type='number'
              min={0}
              value={isi}
              onChange={(event) => isiChangeHandler(event.target.value, index)}
            />
            <br />
          </div>
        )
      })}

      <br />

      <label htmlFor='nofTrains'>Number of trains </label>
      <input
        name='nofTrains'
        type='number'
        min={0}
        value={nofTrains}
        onChange={(event) => setNofTrains(Number(event.target.value))}
      />
      <br />
      <br />

      <h2>Channels</h2>
      <ChannelsContainer>
        {channels.map((channel) => (
          <Channel key={`channel-${channel.channelIndex}`} {...channel} />
        ))}
      </ChannelsContainer>

      <br />
      <button onClick={startSequence}>Start sequence</button>
    </div>
  )
}

const ChannelsContainer = styled.div``

export default PulseSequenceConfiguration
