import React, { useState } from 'react'
import Eye from '../Eye'
import styled from 'styled-components'

export const EyeCell = ({
  value: initialValue,
  row: { index },
  column: { id },
  updateData, // This is a custom function that we supplied to our table instance
}: any) => {
  const [visible, setVisible] = useState(initialValue)

  const onClick = (event: any) => {
    event.stopPropagation()
    const newVisible = !visible
    setVisible(newVisible)
    updateData(index, id, newVisible, true)
  }

  // If the initialValue is changed external, sync it up with our state
  React.useEffect(() => {
    setVisible(visible)
  }, [visible])

  return (
    <EyeButton onClick={onClick}>
      <Eye visible={visible} />
    </EyeButton>
  )
}

const EyeButton = styled.button`
  all: unset;
`
