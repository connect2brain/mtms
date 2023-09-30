import React, { useState } from 'react'
import styled from 'styled-components'

const ControlsContainer = styled.div`
  display: flex;
  flex-direction: column;
  gap: 10px;
`

const ControlBox = styled.div`
  border: 4px solid #e0e0e0;
  padding: 10px;
  border-radius: 5px;
  margin-right: 8px;
  display: flex;
  flex-direction: column;
  align-items: center;
`

const TopLabel = styled.div`
  font-weight: bold;
  margin-bottom: 8px;
  text-align: center;
  width: 100%;
`

const StyledRow = styled.div`
  display: flex;
  align-items: center;
  justify-content: center;
  margin-bottom: 8px;
  width: 100%;
  flex-wrap: wrap;
`

const StyledNarrowInput = styled.input`
  width: 35px;
  margin-right: 8px;
  padding: 4px;
`

const StyledButton = styled.button`
  padding: 8px 15px;
  border-radius: 4px;
  border: none;
  background-color: #007BFF;
  color: white;
  cursor: pointer;
  transition: background-color 0.3s;

  &:hover {
    background-color: #0056b3;
  }
`

const CircleButton = styled(StyledButton)`
  margin-right: 8px;
`

const StyledInput = styled.input`
  padding: 5px;
  border-radius: 4px;
  border: 1px solid #ccc;
`

const StyledLabel = styled.label`
  margin-right: 8px;
`

interface GridControlsProps {
  onShapeSelected: (shape: 'circle' | 'square', size: number) => void
  onReset: () => void
  onDecimate: (rate: number) => void;
}

export const GridControls: React.FC<GridControlsProps> = ({
    onShapeSelected,
    onReset,
    onDecimate
  }) => {
    const [shapeSize, setShapeSize] = useState<number>(1)
    const [decimationRate, setDecimationRate] = useState<number>(2)

    return (
      <ControlsContainer>
        <ControlBox>
          <TopLabel>Shape</TopLabel>
          <StyledRow>
            <StyledLabel>Size:</StyledLabel>
            <StyledNarrowInput
              type="number"
              value={shapeSize}
              onChange={e => setShapeSize(Math.max(1, parseInt(e.target.value)))}
            />
          </StyledRow>
          <StyledRow>
            <CircleButton onClick={() => onShapeSelected('circle', shapeSize)}>Circle</CircleButton>
            <StyledButton onClick={() => onShapeSelected('square', shapeSize)}>Square</StyledButton>
          </StyledRow>
        </ControlBox>

        <ControlBox>
          <TopLabel>Decimation</TopLabel>
          <StyledRow>
            <StyledLabel>Rate:</StyledLabel>
            <StyledNarrowInput
              type="number"
              value={decimationRate}
              onChange={e => setDecimationRate(Math.max(1, parseInt(e.target.value)))}
            />
          </StyledRow>
          <StyledRow>
            <StyledButton onClick={() => onDecimate(decimationRate)}>Decimate</StyledButton>
          </StyledRow>
        </ControlBox>
        <ControlBox>
          <StyledButton onClick={onReset}>Reset grid</StyledButton>
        </ControlBox>
      </ControlsContainer>
  )
    }
