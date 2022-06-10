import React, { useEffect, useState } from 'react'
import styled from 'styled-components'
import useStore from 'providers/state'

const SelectableTargetTableRow = (props: any) => {
  const { index, updateData, isTarget } = props
  const { targets } = useStore()
  const [selected, setSelected] = useState(isTarget ? targets[index].selected : false)

  useEffect(() => {
    if (isTarget) {
      setSelected(targets[index].selected)
    }
  }, [targets[index].selected])

  const onClick = (event: any) => {
    console.log(targets, selected)
    event.preventDefault()

    if (isTarget) {
      updateData(index, 'selected', !selected, true)
    } else {
      console.log('clicked a sequence, not implemented')
    }
  }

  return (
    <TableRow {...props} selected={selected} onClick={onClick}>
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

export default SelectableTargetTableRow
