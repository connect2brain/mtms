import React, { useEffect, useState } from 'react'
import { eegDataSubscriber } from 'services/ros'
import { EegBatch, EegBatchMessage, EegDatapoint, EegDatapointMessage } from 'types/eeg'
import { useAppDispatch, useAppSelector } from 'providers/reduxHooks'
import { addBatch, addEegDatapoint, setEeg } from 'reducers/eegReducer'
import { EegChart } from 'components/EegChart'
import { EegChartSteaming } from '../components/EegChartStreaming'

const DataVisualize = () => {
  const { eeg } = useAppSelector((state) => state.eegData)

  const [latestBatch, setLatestBatch] = useState<EegDatapoint[]>([])

  const dispatch = useAppDispatch()

  useEffect(() => {
    console.log('Subscribing to eeg data')
    eegDataSubscriber.subscribe(newEegBatch)
  }, [])

  const newEegBatch = (message: EegBatchMessage) => {
    const datapoints: EegDatapoint[] = message.batch
    const newData = eeg.concat(datapoints)
    //dispatch(addBatch(newData))
    setLatestBatch(message.batch)
  }

  const c3 = () => {
    return latestBatch.map((datapoint) => {
      return {
        y: datapoint.channel_datapoint[4],
        x: datapoint.time,
      }
    })
  }

  return (
    <div>
      Eeg length: {eeg.length}
      <EegChartSteaming data={c3()} />
    </div>
  )
}

export default DataVisualize
