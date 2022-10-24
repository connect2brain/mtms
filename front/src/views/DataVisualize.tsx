import React, { useEffect, useState } from 'react'
import { eegDataSubscriber, triggerSubscriber } from 'services/ros'
import { EegBatch, EegBatchMessage, EegDatapoint, EegDatapointMessage, EegTriggerMessage } from 'types/eeg'
import { useAppDispatch, useAppSelector } from 'providers/reduxHooks'
import { addBatch, addEegDatapoint, addEegTrigger, setEeg } from 'reducers/eegReducer'
import { Datapoint, EegChartSteaming } from '../components/EegChartStreaming'
import { WebGLPlot } from '../components/WebGLPlot'

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
    eegDataSubscriber.subscribe(newEegBatchWebGL)
    triggerSubscriber.subscribe(newTrigger)
  }, [])

  const c3 = (datapoint: number[]) =>
    datapoint[4] - 0.25 * (datapoint[21] + datapoint[23] + datapoint[25] + datapoint[27])

  const newEegBatch = (message: EegBatchMessage) => {
    const mappedData = message.batch.map((point) => {
      const time = point.time
      return {
        y: c3(point.eeg_channels),
        x: Math.round(time),
      }
    })

    setLatestBatch(mappedData)
  }

  const newEegBatchWebGL = (message: EegBatchMessage) => {
    let max = 0
    let s = 0
    const data = []
    for (let i = 0; i < message.batch.length; i++) {
      const point = message.batch[i]
      const filtered = c3(point.channel_datapoint)
      if (filtered> max) {
        max = filtered
      }
      s += filtered
      data.push(filtered)
    }

    const avg = s / message.batch.length

    const scaledData = data.map(point => {
      return {
        x: 0,
        y: (point - avg) / max * 50
      }
    })

    setLatestBatch(scaledData)
  }

  const newTrigger = (message: EegTriggerMessage) => {
    //console.log('Trigger received')
    setTrigger({
      y: 100000,
      x: message.time,
    })
  }

  return (
    <div>
      {/*<EegChartSteaming eegData={latestBatch} triggerData={trigger} />*/}
      <WebGLPlot eegData={latestBatch} triggerData={trigger} />
    </div>
  )
}

export default DataVisualize
