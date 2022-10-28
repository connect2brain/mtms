import React, { useEffect, useState } from 'react'
import { eegDataSubscriber, triggerSubscriber } from 'services/ros'
import { EegBatchMessage, EegTriggerMessage, MTMSEvent, MTMSEventMessage } from 'types/eeg'
import { Datapoint, DatapointWithEventType, EegChartSteaming } from '../components/EegChartStreaming'
import { WebGLPlot } from '../components/WebGLPlot'
import styled from 'styled-components'
import { eventSubscriber } from '../services/experiment'
import { objectKeysToCamelCase } from '../utils'

const DataVisualizeWebGL = () => {
  const [minY, setMinY] = useState<number>(3550)
  const [maxY, setMaxY] = useState<number>(3620)

  const [latestBatch, setLatestBatch] = useState<Datapoint[]>([])

  const [latestEvent, setLatestEvent] = useState<MTMSEvent>()

  const [events, setEvents] = useState<MTMSEvent[]>([])
  const [eventPoints, setEventPoints] = useState<Datapoint[]>([])

  const [latestTimestamps, setLatestTimestamps] = useState<number[]>([])

  useEffect(() => {
    console.log('Subscribing to eeg data')
    eegDataSubscriber.subscribe(newEegBatchWebGL)
    eventSubscriber.subscribe(newEvent)
  }, [])

  const c3 = (datapoint: number[]) =>
    datapoint[4] - 0.25 * (datapoint[21] + datapoint[23] + datapoint[25] + datapoint[27])

  const newEegBatchWebGL = (message: EegBatchMessage) => {
    let max = 0
    let s = 0
    const data = []
    const eegTimestamps: number[] = []
    for (let i = 0; i < message.batch.length; i++) {
      const point = message.batch[i]
      eegTimestamps.push(point.time)

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
        //y: (-1 - 1) * ((point - minY) / maxY + minY) + minY,
      }
    })

    setLatestTimestamps(eegTimestamps)
    setLatestBatch(scaledData)
  }

  useEffect(() => {
    const newEventData = latestTimestamps.map((p) => {
      return {
        x: 0,
        y: 0,
      }
    })

    const newEvents: MTMSEvent[] = []

    for (let j = 0; j < events.length; j++) {
      const event = events[j]
      let removeThisEvent = false
      for (let i = 0; i < latestTimestamps.length; i++) {
        const ts = latestTimestamps[i]
        if (event.timeUs < ts) {
          newEventData[i].y = 1
          removeThisEvent = true
          break
        }
      }

      if (!removeThisEvent) {
        newEvents.push(event)
      }
    }

    setEvents(() => newEvents)
    setEventPoints(newEventData)
  }, [latestTimestamps])

  useEffect(() => {
    if (latestEvent) {
      const newEvents = [...events, latestEvent]
      setEvents(() => newEvents)
    }
  }, [latestEvent])

  const newEvent = (message: MTMSEventMessage) => {
    const camelCased: MTMSEvent = objectKeysToCamelCase(message)

    setLatestEvent(camelCased)
  }

  return (
    <ChartContainer>
      <label htmlFor='chart-max-input'>Y max: </label>
      <AxisLimit
        defaultValue={maxY}
        type='number'
        id='chart-max-input'
        onChange={(event) => setMaxY(Number(event.target.value))}
      />
      <br />
      <label htmlFor='chart-min-input'>Y min: </label>
      <AxisLimit
        defaultValue={minY}
        type='number'
        id='chart-min-input'
        onChange={(event) => setMinY(Number(event.target.value))}
      />
      <WebGLPlot eegData={latestBatch} pulseData={eventPoints} />
    </ChartContainer>
  )
}

const ChartContainer = styled.div``

const AxisLimit = styled.input`
  max-width: 50px;
`

export default DataVisualizeWebGL
