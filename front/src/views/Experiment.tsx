import React, { useState } from 'react'
import useStore from '../providers/state'
import PulseSequenceConfiguration from './PulseSequenceConfiguration'

const Experiment = () => {
  const { description, setDescription } = useStore((state) => state)

  const startSequence = (event: any) => {
    const sequenceData = {}

    console.log('starting sequence:', sequenceData)
  }

  return (
    <div>
      <h2>Pulse sequence configuration</h2>
      <label htmlFor='description'>Description </label>
      <input name='description' type='text' value={description} onChange={(event) => setDescription(event.target.value)} />

      <br />

      <PulseSequenceConfiguration />

      <button onClick={startSequence}>Start sequence</button>
    </div>
  )
}

export default Experiment
