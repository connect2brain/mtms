import React, { ReactNode, useEffect, useState } from 'react'
import styled from 'styled-components'
import useStore from 'providers/state'
import { getSequenceIndexFromRowId } from 'utils'

interface Props {
  index: number
  isTarget: boolean
  rowId: string
  children: ReactNode
}

const SelectableSequenceTableRow = (props: Props) => {
  const { index, isTarget, rowId } = props

  const { sequences, setSequences, targets } = useStore()

  const sequenceIndex = isTarget ? getSequenceIndexFromRowId(rowId) : index

  const sequence = sequences[sequenceIndex]
  const [selected, setSelected] = useState(isTarget ? sequence.pulses[index]?.selected : sequence.selected)

  /* is still needed? Yep, if store selected changes, this should be changed too TODO
    useEffect(() => {
      if (isTarget) {
        setSelected(targets[index].selected)
      }
    }, [targets[index].selected])
  */

  const onClick = (event: any) => {
    event.preventDefault()

    if (isTarget) {
      setSelected(!selected)
    } else {
      const newSequence = {
        ...sequence,
        selected: !sequence.selected,
      }
      const newSequences = sequences.filter((seq) => seq.name !== sequence.name)
      newSequences.splice(sequenceIndex, 0, newSequence)
      setSequences(newSequences)
      setSelected(!sequence.selected)
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

export default SelectableSequenceTableRow
