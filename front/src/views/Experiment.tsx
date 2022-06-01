import React, { useState } from 'react'
import PulseSequenceConfiguration from './PulseSequenceConfiguration'
import useStore from '../providers/state'
import { ExperimentMessage } from '../types/pulseSequence'
import { startExperimentService } from '../services/ros'
import ROSLIB from 'roslib'
import { objectKeysToSnakeCase } from '../utils'
import styled from 'styled-components'

const Experiment = () => {
  const { description, setDescription, channels, iti, ibi, nofBurstsInTrains, nofPulsesInBursts, nofTrains, isis } =
    useStore((state) => state)

  const [statusMessage, setStatusMessage] = useState<string>('')
  const [status, setStatus] = useState<'OK' | 'ERROR'>('OK')

  const startSequence = (event: any) => {
    setStatus('OK')
    setStatusMessage('Starting sequence...')

    const channelData = channels
      .filter((channel) => channel.enabled)
      .map((fullChannel) => {
        const { enabled, ...channel } = fullChannel
        return channel
      })

    const messageData: ExperimentMessage = {
      description,
      pulseSequence: {
        iti,
        ibi,
        nofBurstsInTrains,
        nofPulsesInBursts,
        nofTrains,
        isis,
        channelInfo: channelData,
      },
    }

    console.log('starting sequence:', messageData)

    const messageDataSnakeCase = objectKeysToSnakeCase(messageData)

    const message = new ROSLIB.Message(messageDataSnakeCase)
    const request = new ROSLIB.ServiceRequest({
      experiment: message,
    })

    startExperimentService.callService(
      request,
      (response) => {
        if (!response.success) {
          console.error('FAILED TO START SEQUENCE, response', response)
          setStatus('ERROR')
          setStatusMessage(`Failed to start sequence! Error: ${response.status}`)
        } else {
          console.log('Started sequence')
          console.log(response.sequence)
          setStatus('OK')
          setStatusMessage('Started sequence')
        }
      },
      (error) => {
        console.error(error)
        setStatus('ERROR')
        setStatusMessage(`Failed to start sequence! Error: ${error}`)
      },
    )
  }

  return (
    <div>
      <h1>Experiment</h1>
      <label htmlFor='description'>Description </label>
      <input
        name='description'
        type='text'
        value={description}
        onChange={(event) => setDescription(event.target.value)}
      />

      <br />

      <PulseSequenceConfiguration />

      <button onClick={startSequence}>Start sequence</button>
      <div>
        <StatusMessage status={status}>{statusMessage}</StatusMessage>
      </div>
    </div>
  )
}

const StatusMessage = styled.p<{
  status: 'OK' | 'ERROR'
}>`
  color: ${(p) => (p.status === 'OK' ? p.theme.colors.primary : p.theme.colors.error)};
`

export default Experiment
