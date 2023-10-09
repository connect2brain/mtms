import React, { useState } from 'react'
import styled from 'styled-components'

import { ToggleSwitch } from 'components/Experiment/ToggleSwitch'

const DelayLabel = styled.label<{ isEnabled: boolean }>`
    width: 500px;
    font-size: 11px;
    font-weight: bold;
    text-align: right;
    color: ${props => props.isEnabled ? 'black' : 'gray'};
`

const DelayInput = styled.input<{ isEnabled: boolean }>`
    margin-top: 0px;
    pointer-events: ${props => props.isEnabled ? 'auto' : 'none'};
    margin-right: 20px;
    width: 30px;
    background-color: ${props => props.isEnabled ? 'white' : '#f0f0f0'};
    color: ${props => props.isEnabled ? 'black' : 'gray'};
`

export const TriggerSelector: React.FC<{
    enabled: boolean
    enabledHandler: (value: boolean) => void
    delay: number
    delayHandler: (value: number) => void
  }> = ({ enabled, enabledHandler, delay, delayHandler }) => {

    const [inputValue, setInputValue] = useState(String(delay))

    const handleChangeDelay = (event: React.ChangeEvent<HTMLInputElement>) => {
        setInputValue(event.target.value)

        const value = Number(event.target.value)
        if (!isNaN(value) && value >= -99 && value <= 99) {
            delayHandler(value)
        }
    }

    return (
      <>
        <ToggleSwitch
          type="flat"
          checked={enabled}
          onChange={enabledHandler}
        />
        <DelayLabel isEnabled={enabled}>Delay (ms)</DelayLabel>
        <DelayInput
          isEnabled={enabled}
          type="text"
          value={inputValue}
          min={-99}
          max={99}
          onChange={handleChangeDelay}
        />
      </>
    )
  }
