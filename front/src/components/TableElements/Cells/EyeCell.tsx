import React, { useState } from 'react'
import Eye from '../../Eye'
import styled from 'styled-components'
import { CellProps } from 'types/table'
import { useAppSelector } from 'providers/reduxHooks'
import { updateTargetInRos } from 'services/target'

export const EyeCell = ({ value: initialValue, row, column }: CellProps) => {
  const [visible, setVisible] = useState(initialValue)
  const { targets } = useAppSelector((state) => state.targets)

  const onClick = (event: any) => {
    event.stopPropagation()
    const newVisible = !visible
    setVisible(newVisible)
    if (column.id) {
      const target = targets[row.index]
      updateTargetInRos(target, column.id, newVisible, true)
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
