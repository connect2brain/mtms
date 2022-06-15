import React, { ReactNode, useRef, useState } from 'react'
import styled from 'styled-components'
import useStore from 'providers/state'
import { getSequenceIndexFromRowId } from 'utils'
import { DragSourceMonitor, useDrag, useDrop } from 'react-dnd'
import Dots from '../Dots'

interface Props {
  index: number
  isTarget: boolean
  rowId: string
  children: ReactNode
}

interface DragItem {
  type: 'row'
  index: number
  sequenceIndex: number
}

const SelectableSequenceTableRow = (props: Props) => {
  const { index, isTarget, rowId } = props

  const { sequences, setSequences, targets } = useStore()

  const sequenceIndex = isTarget ? getSequenceIndexFromRowId(rowId) : index

  const sequence = sequences[sequenceIndex]
  const [selected, setSelected] = useState(isTarget ? sequence.pulses[index]?.selected : sequence.selected)

  const dropRef = useRef<HTMLTableRowElement>(null)
  const dragRef = useRef<HTMLTableCellElement>(null)

  /* is still needed? Yep, if store selected changes, this should be changed too TODO
          useEffect(() => {
            if (isTarget) {
              setSelected(targets[index].selected)
            }
          }, [targets[index].selected])
        */

  const moveRow = (dragIndex: number, hoverIndex: number) => {
    const newPulses = [...sequence.pulses]

    newPulses.splice(hoverIndex, 0, newPulses.splice(dragIndex, 1)[0])

    const newSequence = {
      ...sequence,
      pulses: newPulses,
    }

    const newSequences = sequences.filter((seq) => seq.name !== sequence.name)
    newSequences.splice(sequenceIndex, 0, newSequence)
    setSequences(newSequences)

    //setTargets(newTargets)
  }

  const [, drop] = useDrop({
    accept: 'row',
    hover: (item: DragItem, monitor) => {
      //disable moving targets across sequences
      if (item.sequenceIndex !== sequenceIndex) return

      if (!dropRef.current) {
        return
      }
      const dragIndex: number = item.index
      const hoverIndex: number = index
      // Don't replace items with themselves
      if (dragIndex === hoverIndex) {
        return
      }
      // Determine rectangle on screen
      const hoverBoundingRect = dropRef.current.getBoundingClientRect()
      // Get vertical middle
      const hoverMiddleY = (hoverBoundingRect.bottom - hoverBoundingRect.top) / 2

      // Determine mouse position
      const clientOffset = monitor.getClientOffset()
      if (!clientOffset || !clientOffset.y) {
        console.log('no clientOffset for row', index)
        return
      }

      // Get pixels to the top
      const hoverClientY = clientOffset.y - hoverBoundingRect.top

      // Only perform the move when the mouse has crossed half of the items height
      // When dragging downwards, only move when the cursor is below 50%
      // When dragging upwards, only move when the cursor is above 50%
      // Dragging downwards
      if (dragIndex < hoverIndex && hoverClientY < hoverMiddleY) {
        return
      }
      // Dragging upwards
      if (dragIndex > hoverIndex && hoverClientY > hoverMiddleY) {
        return
      }
      // Time to actually perform the action
      moveRow(dragIndex, hoverIndex)
      // Note: we're mutating the monitor item here!
      // Generally it's better to avoid mutations,
      // but it's good here for the sake of performance
      // to avoid expensive index searches.
      item.index = hoverIndex
    }
  })

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

  const [{ isDragging }, drag, preview] = useDrag({
    item: {
      type: 'row',
      index,
      sequenceIndex,
    },
    type: 'row',
    collect: (monitor: DragSourceMonitor) => ({
      isDragging: monitor.isDragging(),
    }),
  })

  preview(drop(dropRef))
  drag(dragRef)

  return (
    <TableRow {...props} selected={selected} onClick={onClick} ref={dropRef}>
      {isTarget ? (
        <td ref={dragRef}>
          <Dots />
        </td>
      ) : (
        <td></td>
      )}

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
