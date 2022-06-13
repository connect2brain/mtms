import React, { useEffect, useState } from 'react'
import Expand from '../../Expand'
import EditableCell, { EditableSequenceTableCell } from './EditableCell'
import useStore from 'providers/state'
import { CellProps } from 'types/table'

const ExpandableCell = ({ value, row, column }: CellProps) => {
  const { canExpand, depth } = row
  const { expandedSequences, setExpandedSequences } = useStore()

  const [isExpanded, setIsExpanded] = useState(row.isExpanded)

  useEffect(() => {
    setIsExpanded(row.isExpanded)
  }, [row.isExpanded])

  const handleClickExpand = (event: any) => {
    event.stopPropagation()
    row.toggleRowExpanded()

    setExpandedSequences({
      ...expandedSequences,
      [row.index]: !isExpanded,
    })
  }
  const expandElement = canExpand ? <Expand onClick={handleClickExpand} expanded={isExpanded} /> : null

  return (
    <EditableSequenceTableCell
      value={value}
      row={row}
      column={column}
      expandElement={expandElement}
      whiteSpace={depth > 0}
    />
  )
}

export default ExpandableCell
