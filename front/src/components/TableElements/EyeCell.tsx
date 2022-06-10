import React, { useState } from 'react'
import Eye from '../Eye'
import styled from 'styled-components'
import { CellProps } from 'types/table'

interface EyeCellProps extends CellProps {
  updateData: (rowIndex: number, columnName: string, value: any, toggle: boolean) => void
}

export const EyeCell = ({ value: initialValue, row, column, updateData }: EyeCellProps) => {
  const [visible, setVisible] = useState(initialValue)

  const onClick = (event: any) => {
    event.stopPropagation()
    const newVisible = !visible
    setVisible(newVisible)
    if (column.id) {
      updateData(row.index, column.id, newVisible, true)
    }
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
