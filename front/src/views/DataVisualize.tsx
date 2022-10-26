import React, { useEffect, useState } from 'react'
import { eegDataSubscriber, triggerSubscriber } from 'services/ros'
import { EegBatchMessage, EegTriggerMessage, MTMSEvent, MTMSEventMessage } from 'types/eeg'
import { Datapoint, DatapointWithEventType, EegChartSteaming } from '../components/EegChartStreaming'
import { WebGLPlot } from '../components/WebGLPlot'
import styled from 'styled-components'
import { eventSubscriber } from '../services/experiment'
import { objectKeysToCamelCase } from '../utils'

const DataVisualize = () => {
  const [latestBatch, setLatestBatch] = useState<Datapoint[]>([])

  const [trigger, setTrigger] = useState<Datapoint>({
    y: 1,
    x: 1,
  })

  const [latestEvent, setLatestEvent] = useState<DatapointWithEventType>({
    y: 1,
    x: 1,
    eventType: 0,
  })

  useEffect(() => {
    console.log('Subscribing to eeg data')
    eegDataSubscriber.subscribe(newEegBatch)
    triggerSubscriber.subscribe(newTrigger)
    eventSubscriber.subscribe(newEvent)
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
      if (filtered > max) {
        max = filtered
      }
      s += filtered
      data.push(filtered)
    }

    const avg = s / message.batch.length

    const scaledData = data.map((point) => {
      return {
        x: 0,
        y: ((point - avg) / max) * 50,
      }
    })

    setLatestBatch(scaledData)
  }

  const newTrigger = (message: EegTriggerMessage) => {
    setTrigger({
      y: 100000,
      x: message.time,
    })
  }

  const newEvent = (message: MTMSEventMessage) => {
    const camelCased: MTMSEvent = objectKeysToCamelCase(message)
    const timeInSeconds = Math.round(camelCased.timeUs / 1000000)

    console.log(`message.time_us: ${message.time_us}, timeInSeconds: ${timeInSeconds}`)

    setLatestEvent({
      y: 100000,
      x: timeInSeconds,
      eventType: camelCased.eventType,
    })
  }

  return (
    <ChartContainer>
      <EegChartSteaming eegData={latestBatch} latestEvent={latestEvent} />
    </ChartContainer>
  )
}

const ChartContainer = styled.div``

export default DataVisualize
