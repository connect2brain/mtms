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
  const [startTime, setStartTime] = useState(new Date())

  const [trigger, setTrigger] = useState<Datapoint>({
    y: 1,
    x: 1,
  })

  const [latestEvent, setLatestEvent] = useState<DatapointWithEventType>({
    y: 1,
    x: 1,
    eventType: 0,
  })

  const [events, setEvents] = useState<MTMSEvent[]>([])

  useEffect(() => {
    console.log('Subscribing to eeg data')
    eegDataSubscriber.subscribe(newEegBatch)
    eventSubscriber.subscribe(newEvent)
  }, [])

  const c3 = (datapoint: number[]) =>
    datapoint[4] - 0.25 * (datapoint[21] + datapoint[23] + datapoint[25] + datapoint[27])

  const newEegBatch = (message: EegBatchMessage) => {
    const mappedData = message.batch.map((point) => {
      return {
        y: c3(point.eeg_channels),
        x: startTime.getTime() + point.time * 1000,
      }
    })

    setLatestBatch(mappedData)
  }

  const newTrigger = (message: EegTriggerMessage) => {
    setTrigger({
      y: 100000,
      x: message.time,
    })
  }

  const newEvent = (message: MTMSEventMessage) => {
    const camelCased: MTMSEvent = objectKeysToCamelCase(message)
    const time = camelCased.time

    setLatestEvent({
      y: 100000,
      x: time,
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
