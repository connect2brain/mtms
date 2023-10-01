import React, { useState } from 'react'
import styled from 'styled-components'

import { SmallerTitle, ExperimentInput } from './Styles'

interface IntensitySelectorProps {
  min: number
  max: number
  maximumIntensity: number
  onValueChange: (value: number) => void
}

const MaximumIntensityLabel = styled.span`
  position: absolute;
  font-size: 12px; /* Adjust as needed */
  color: red;
  right: 80%;
  transform: translateY(-50%);
`

const MaximumIntensityLine = styled.div<{ top: string }>`
  position: absolute;
  width: 25%;
  height: 2px;
  background-color: red;
  top: ${props => props.top};
  right: 50%;
  z-index: 1;
`

const IntensitySelectorContainer = styled.div`
  display: flex;
  flex-direction: column;
  align-items: center;
  position: relative;
  width: 90px;
`

const IntensityLabel = styled.span`
  font-size: 0.85em;
  font-weight: bold;
  margin-top: 20px;
  margin-bottom: 10px;
`

export const IntensitySelector: React.FC<IntensitySelectorProps> = ({ min, max, maximumIntensity, onValueChange }) => {
  const [selectedIntensity, setSelectedIntensity] = useState<number>((min + max) / 2)

  const handleChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const value = Number(event.target.value)
    setSelectedIntensity(value)
    onValueChange(value)
  }

  const sliderHeight = 350
  const maximumIntensityPositionInPixels = sliderHeight * (1 - (maximumIntensity - min) / (max - min))
  const maximumIntensityPosition = `${maximumIntensityPositionInPixels}px`

  return (
    <div>
      <SmallerTitle>Intensity</SmallerTitle>
      <IntensitySelectorContainer>
        <MaximumIntensityLine top={maximumIntensityPosition} />
        <input
          className="intensitySlider"
          type="range"
          min={min}
          max={max}
          value={selectedIntensity}
          onChange={handleChange}
          style={{
            writingMode: 'vertical-lr',
            WebkitAppearance: 'slider-vertical', /* WebKit */
            width: '8px',
            height: '350px',
            zIndex: 2
          }}
        />
        <MaximumIntensityLabel style={{ top: maximumIntensityPosition }}>
          <b>Max:</b> {maximumIntensity}
        </MaximumIntensityLabel>
        <IntensityLabel>Intensity (V/m):</IntensityLabel>
        <ExperimentInput
          type="number"
          value={selectedIntensity}
          min={min}
          max={max}
          onChange={handleChange}
        />
      </IntensitySelectorContainer>
    </div>
  )
}
