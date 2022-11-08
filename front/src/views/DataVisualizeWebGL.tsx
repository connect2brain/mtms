import React, {useEffect, useRef, useState} from 'react'
import {eegDataSubscriber, triggerSubscriber} from 'services/ros'
import {EegBatchMessage, EegTriggerMessage, MTMSEvent, MTMSEventMessage} from 'types/eeg'
import {Datapoint, DatapointWithEventType, EegChartSteaming} from '../components/EegChartStreaming'
import {WebGLPlot} from '../components/WebGLPlot'
import styled from 'styled-components'
import {eventSubscriber} from '../services/experiment'
import {objectKeysToCamelCase} from '../utils'

const DataVisualizeWebGL = () => {
  const [minY, setMinY] = useState<number>(3550)
  const [maxY, setMaxY] = useState<number>(3620)

  const minYRef = useRef(3550)
  const maxYRef = useRef(3620)

  const targetYMin = -1
  const targetYMax = 1

  const [latestBatch, setLatestBatch] = useState<Datapoint[]>([])

  const [latestEvent, setLatestEvent] = useState<MTMSEvent>()

  const [events, setEvents] = useState<MTMSEvent[]>([])
  const [pulsePoints, setPulsePoints] = useState<Datapoint[]>([])
  const [chargePoints, setChargePoints] = useState<Datapoint[]>([])
  const [dischargePoints, setDischargePoints] = useState<Datapoint[]>([])
  const [signalOutPoints, setSignalOutPoints] = useState<Datapoint[]>([])

  const [latestTimestamps, setLatestTimestamps] = useState<number[]>([])

  useEffect(() => {
    console.log('Subscribing to eeg data')
    eegDataSubscriber.subscribe(newEegBatchWebGL)
    eventSubscriber.subscribe(newEvent)
  }, [])

  const c3 = (datapoint: number[]) =>
    datapoint[4] - 0.25 * (datapoint[21] + datapoint[23] + datapoint[25] + datapoint[27])

  const scaleY = (point: number) =>
    ((point - minYRef.current) / (maxYRef.current - minYRef.current)) * (targetYMax - targetYMin) + targetYMin

  const newEegBatchWebGL = (message: EegBatchMessage) => {
    const data = []
    const eegTimestamps: number[] = []
    for (let i = 0; i < message.batch.length; i++) {
      const point = message.batch[i]
      eegTimestamps.push(point.time)

      const filtered = c3(point.eeg_channels)
      const y = scaleY(filtered)

      const finalPoint = {
        x: 0,
        y,
      }
      data.push(finalPoint)
    }

    setLatestTimestamps(eegTimestamps)
    setLatestBatch(data)
  }

  useEffect(() => {
    const newEventData = latestTimestamps.map((p) => {
      return {
        x: 0,
        y: -1,
      }
    })


    const initData = () => {
      return latestTimestamps.map((p) => {
        return {
          x: 0,
          y: -1,
        }
      })
    }

    const newPulseData = initData()
    const newChargeData = initData()
    const newDischargeData = initData()
    const newSignalOutData = initData()

    const allData = [newPulseData, newChargeData, newDischargeData, newSignalOutData]

    const newEvents: MTMSEvent[] = []

    for (let j = 0; j < events.length; j++) {
      const event = events[j]
      let removeThisEvent = false
      for (let i = 0; i < latestTimestamps.length; i++) {
        const ts = latestTimestamps[i]
        if (event.time < ts) {
          console.log(`event: ${event.eventType}, ${event.time}, ${ts}`)
          allData[event.eventType][i].y = 1
          removeThisEvent = true
          break
        }
      }

      if (!removeThisEvent) {
        newEvents.push(event)
      }
    }

    setEvents(() => newEvents)

    setChargePoints(newChargeData)
    setPulsePoints(newPulseData)
    setDischargePoints(newDischargeData)
    setSignalOutPoints(newSignalOutData)
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

  const updateLimits = (
    limitFunc: React.Dispatch<React.SetStateAction<number>>,
    ref: React.MutableRefObject<number>,
    newLimit: number,
  ) => {
    console.log('new limit:', newLimit)
    limitFunc((oldLimit) => newLimit)
    ref.current = newLimit
  }

  return (
    <ChartContainer>
      <label htmlFor='chart-max-input'>Y max: </label>
      <AxisLimit
        defaultValue={maxY}
        type='number'
        id='chart-max-input'
        onChange={(event) => updateLimits(setMaxY, maxYRef, Number(event.target.value))}
      />
      <br/>
      <label htmlFor='chart-min-input'>Y min: </label>
      <AxisLimit
        defaultValue={minY}
        type='number'
        id='chart-min-input'
        onChange={(event) => updateLimits(setMinY, minYRef, Number(event.target.value))}
      />
      <WebGLPlot
        eegData={latestBatch}
        pulseData={pulsePoints}
        chargeData={chargePoints}
        dischargeData={dischargePoints}
        signalOutData={signalOutPoints}
      />
    </ChartContainer>
  )
}

const ChartContainer = styled.div``

const AxisLimit = styled.input`
  max-width: 50px;
`

export default DataVisualizeWebGL
