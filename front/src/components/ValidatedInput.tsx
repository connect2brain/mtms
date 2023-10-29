import React, { useState, useEffect } from 'react'
import styled from 'styled-components'

const StyledInput = styled.input<{ valid?: boolean }>`
  marginTop: 0px;
  width: 43px;
  margin-right: 40px;
  border: 2px solid ${props => (props.valid ? 'black' : 'red')};
  outline-color: ${props => (props.valid ? 'black' : 'red')};
  background-color: 'white';
  color: 'black';
`

interface ValidatedInputProps {
  value: number
  onChange: (newValue: number) => void
  defaultValue: number // Added a default value prop
  type?: string
  min?: number
  max?: number
  step?: number
  disabled?: boolean
}

export const ValidatedInput: React.FC<ValidatedInputProps> = ({
  value,
  onChange,
  type = 'number',
  min,
  max,
  defaultValue,
  ...props
}) => {
  const [localValue, setLocalValue] = useState<string>(defaultValue.toString())

  const isValueValid = (value: string): boolean => {
    const newValue = parseFloat(value)
    if (isNaN(newValue)) return false
    return (min === undefined || newValue >= min) && (max === undefined || newValue <= max)
  }

  const handleChange = (inputValue: string) => {
    setLocalValue(inputValue)
    if (isValueValid(inputValue)) {
      onChange(parseFloat(inputValue))
    }
  }

  const handleBlur = () => {
    if (!isValueValid(localValue) && value !== undefined) {
      setLocalValue(value.toString())
    } else if (isValueValid(localValue)) {
      onChange(parseFloat(localValue))
    }
  }

  return (
    <StyledInput
      type={type}
      {...props}
      value={localValue}
      valid={isValueValid(localValue)}
      onChange={e => handleChange(e.target.value)}
      onBlur={handleBlur}
    />
  )
}
