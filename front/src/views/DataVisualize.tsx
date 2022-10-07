import React, { useEffect, useState } from 'react'
import { eegDataSubscriber, triggerSubscriber } from 'services/ros'
import { EegBatch, EegBatchMessage, EegDatapoint, EegDatapointMessage, EegTriggerMessage } from 'types/eeg'
import { useAppDispatch, useAppSelector } from 'providers/reduxHooks'
import { addBatch, addEegDatapoint, addEegTrigger, setEeg } from 'reducers/eegReducer'
import { Datapoint, EegChartSteaming } from '../components/EegChartStreaming'

const DataVisualize = () => {
  const { eeg } = useAppSelector((state) => state.eegData)

  const [latestBatch, setLatestBatch] = useState<Datapoint[]>([])

  const [trigger, setTrigger] = useState<Datapoint>({
    y: 1,
    x: 1,
  })

  const dispatch = useAppDispatch()

  useEffect(() => {
    console.log('Subscribing to eeg data')
    eegDataSubscriber.subscribe(newEegBatch)
    triggerSubscriber.subscribe(newTrigger)
  }, [])

  const c3 = (datapoint: number[]) =>
    datapoint[4] - 0.25 * (datapoint[21] + datapoint[23] + datapoint[25] + datapoint[27])

  const newEegBatch = (message: EegBatchMessage) => {
    const mappedData = message.batch.map((point) => {
      const timeInSeconds = point.time / 1000000
      return {
        y: c3(point.channel_datapoint),
        x: Math.round(timeInSeconds),
      }
    })
    setLatestBatch(mappedData)
  }

  const newTrigger = (message: EegTriggerMessage) => {
    console.log(message)
    setTrigger({
      y: 100000,
      x: message.time_us / 1000000,
    })
  }

  return (
    <div>
      <EegChartSteaming eegData={latestBatch} triggerData={trigger} />
    </div>
  )
}

export default DataVisualize
