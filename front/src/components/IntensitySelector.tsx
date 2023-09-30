import React, { useState } from 'react'
import styled from 'styled-components'

interface IntensitySelectorProps {
  min: number
  max: number
  threshold: number
  onValueChange: (value: number) => void
}

const IntensityTitle = styled.h2`
  font-size: 18px;
  text-align: center;
  color: #333;
  margin-bottom: 30px;
  margin-right: 30px;
  font-weight: bold;
`

const ThresholdLabel = styled.span`
  position: absolute;
  font-size: 12px; /* Adjust as needed */
  color: red;
  right: 85%;
  transform: translateY(-50%);
`

const ThresholdLine = styled.div<{ top: string }>`
  position: absolute;
  width: 30%;
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
  width: 70px;
`

const IntensityInput = styled.input`
  marginTop: 0px;
  width: 40px;
`

const IntensityLabel = styled.span`
  font-size: 0.85em;
  font-weight: bold;
  margin-top: 5px;
  margin-bottom: 5px;
`

export const IntensitySelector: React.FC<IntensitySelectorProps> = ({ min, max, threshold, onValueChange }) => {
  const [selectedIntensity, setSelectedIntensity] = useState<number>((min + max) / 2)

  const handleChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const value = Number(event.target.value)
    setSelectedIntensity(value)
    onValueChange(value)
  }

  const sliderHeight = 200
  const thresholdPositionInPixels = sliderHeight * (1 - (threshold - min) / (max - min))
  const thresholdPosition = `${thresholdPositionInPixels}px`

  return (
    <div>
      <IntensityTitle>Intensity</IntensityTitle>
      <IntensitySelectorContainer>
        <ThresholdLine top={thresholdPosition} />
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
            height: '200px',
            zIndex: 2
          }}
        />
        <ThresholdLabel style={{ top: thresholdPosition }}>
          <b>Max:</b> {threshold}
        </ThresholdLabel>
        <IntensityLabel>Intensity (V/m):</IntensityLabel>
        <IntensityInput
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
