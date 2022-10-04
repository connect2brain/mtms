import React, { useEffect, useState } from 'react'
import { eegDataSubscriber } from 'services/ros'
import { EegBatchMessage, EegDatapoint, EegDatapointMessage } from 'types/eeg'
import { useAppDispatch, useAppSelector } from 'providers/reduxHooks'
import { addBatch, addEegDatapoint, setEeg } from 'reducers/eegReducer'
import { EegChart } from 'components/EegChart'

const DataVisualize = () => {
  const { eeg } = useAppSelector((state) => state.eegData)

  const dispatch = useAppDispatch()

  useEffect(() => {
    console.log('Subscribing to eeg data')
    eegDataSubscriber.subscribe(newEegBatch)
  }, [])

  const newEegBatch = (message: EegBatchMessage) => {
    const datapoints: EegDatapoint[] = message.batch
    const newData = eeg.concat(datapoints)
    dispatch(addBatch(newData))
  }

  const c3 = () => {
    return eeg.map((datapoint) => datapoint.channel_datapoint[4])
  }

  return (
    <div>
      Eeg length: {eeg.length}
      <EegChart data={c3()} />
    </div>
  )
}

export default DataVisualize
