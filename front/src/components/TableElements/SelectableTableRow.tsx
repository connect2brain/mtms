import useStore from '../../providers/state'
import React from 'react'
import styled from 'styled-components'

const SelectableTableRow = (props: any) => {
  const { index, updateData } = props
  const { targets, setTargets } = useStore((state) => state)

  const onClick = (event: any) => {
    event.preventDefault()
    console.log('clicked row', index)
    const selected = targets[index].selected

    updateData(index, 'selected', !selected, true)
  }

  return (
    <TableRow {...props} selected={targets[index].selected} onClick={onClick}>
      {props.children}
    </TableRow>
  )
}

const TableRow = styled.tr<{
  selected: boolean
}>`
  border-bottom: 1px solid ${(p) => p.theme.colors.gray};
  background-color: ${(p) => (p.selected ? p.theme.colors.lightgray : p.theme.colors.white)};
`

export default SelectableTableRow
