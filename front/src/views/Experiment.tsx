import React, { useState } from 'react'
import PulseSequenceConfiguration from './PulseSequenceConfiguration'
import useStore from '../providers/state'
import { ExperimentMessage } from '../types/pulseSequence'
import { startExperimentService } from '../services/ros'
import ROSLIB from 'roslib'
import { objectKeysToSnakeCase } from '../utils'

const Experiment = () => {
  const { description, setDescription, channels, iti, ibi, nofBurstsInTrains, nofPulsesInBursts, nofTrains, isis } =
    useStore((state) => state)

  const startSequence = (event: any) => {
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
        } else {
          console.log('Started sequence')
        }
      },
      (error) => {
        console.error(error)
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
    </div>
  )
}

export default Experiment
