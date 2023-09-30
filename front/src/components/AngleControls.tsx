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
  width: 45px;
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

const StyledLabel = styled.label`
  margin-right: 8px;
  margin-bottom: 5px;
`

export const AngleControls: React.FC<{
    onFillAngles: (startAngle: number, increment: number) => void
    onReset: () => void
  }> = ({ onFillAngles, onReset }) => {
    const [startAngle, setStartAngle] = useState<number>(0)
    const [increment, setIncrement] = useState<number>(30)

    return (
      <ControlsContainer>
        <ControlBox>
          <TopLabel>Angles</TopLabel>
          <StyledRow>
            <StyledLabel>Start Angle:</StyledLabel>
            <StyledNarrowInput
              type="number"
              value={startAngle}
              onChange={e => setStartAngle(Math.max(0, Math.min(350, parseInt(e.target.value))))}
            />
          </StyledRow>
          <StyledRow>
            <StyledLabel>Increment:</StyledLabel>
            <StyledNarrowInput
              type="number"
              value={increment}
              onChange={e => setIncrement(Math.max(0, Math.min(350, parseInt(e.target.value))))}
            />
          </StyledRow>
          <StyledRow>
            <StyledButton onClick={() => onFillAngles(startAngle, increment)}>
              Fill
            </StyledButton>
          </StyledRow>
        </ControlBox>

        <ControlBox>
          <StyledButton onClick={onReset}>Reset angles</StyledButton>
        </ControlBox>
      </ControlsContainer>
    )
  }
