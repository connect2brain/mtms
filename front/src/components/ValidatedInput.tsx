import React, { useState } from 'react'
import styled from 'styled-components'

const StyledInput = styled.input`
  marginTop: 0px;
  width: 43px;
  margin-right: 40px;
  border: 2px solid black;
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
  const [tempValue, setTempValue] = useState<string>(value.toString())

  const handleInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    setTempValue(e.target.value)
  }

  const handleInputBlur = () => {
    let numericValue = Number(tempValue)

    // If value is non-numeric or invalid, revert to default value
    if (isNaN(numericValue)) {
      numericValue = defaultValue
    }
    
    // Boundary checks
    if (min !== undefined && numericValue < min) {
      numericValue = min
    }

    if (max !== undefined && numericValue > max) {
      numericValue = max
    }

    onChange(numericValue)
    setTempValue(numericValue.toString()) // Update the temporary value with the clipped or default value
  }

  return (
    <StyledInput
      type={type}
      {...props}
      value={tempValue}
      onChange={handleInputChange}
      onBlur={handleInputBlur}
    />
  )
}
