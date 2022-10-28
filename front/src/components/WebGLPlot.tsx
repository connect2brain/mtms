import React, { useEffect, useRef, useState } from 'react'
import { WebglPlot, WebglLine, ColorRGBA } from 'webgl-plot'
import {Datapoint, EegChartProps} from './EegChartStreaming'
import styled from 'styled-components'

type WebGLPlotProps = {
  eegData: Datapoint[]
  pulseData: Datapoint[]
}

export const WebGLPlot = ({ eegData, pulseData }: WebGLPlotProps) => {
  const canvasMain = useRef<HTMLCanvasElement>(null)

  const [webGLPlot, setWebGLPlot] = useState<WebglPlot>()
  const [eegLine, setEegLine] = useState<WebglLine>()
  const [pulseLine, setPulseLine] = useState<WebglLine>()
  const [triggerLine, setTriggerLine] = useState<WebglLine>()

  useEffect(() => {
    if (canvasMain.current) {
      const devicePixelRatio = window.devicePixelRatio || 1
      canvasMain.current.width = canvasMain.current.clientWidth * devicePixelRatio
      canvasMain.current.height = canvasMain.current.clientHeight * devicePixelRatio

      const webglp = new WebglPlot(canvasMain.current)
      setWebGLPlot(webglp)
      const numX = 2500

      const line1 = new WebglLine(new ColorRGBA(1, 0, 0, 1), numX)
      const line2 = new WebglLine(new ColorRGBA(1, 0, 0, 1), numX)
      webglp.addLine(line1)
      webglp.addLine(line2)

      line1.arrangeX()
      line2.arrangeX()

      setWebGLPlot(webglp)
      setEegLine(line1)
      setPulseLine(line2)
    }
  }, [])

  useEffect(() => {
    //console.log('in webgl use effect eeg data')
    let id = 0
    let renderPlot = () => {
      //console.log('in render plot')
      const y = new Float32Array(eegData.map(datapoint => datapoint.y))
      //console.log(y)
      eegLine?.shiftAdd(y)
      //eegLine?.
      //id = requestAnimationFrame(renderPlot)
      webGLPlot?.update()
    }

    id = requestAnimationFrame(renderPlot)

    return () => {
      // eslint-disable-next-line @typescript-eslint/no-empty-function
      renderPlot = () => {}
      cancelAnimationFrame(id)
    }

  }, [eegData])

  return (
    <div>
      <StyledCanvas ref={canvasMain} />
    </div>
  )
}

const StyledCanvas = styled.canvas`
  width: 100%;
  height: 70vh;
  border: 1px solid black;
`
