import React, { useEffect, useState } from 'react'
import Expand from '../Expand'
import EditableCell, {EditableSequenceTableCell} from './EditableCell'
import useStore from 'providers/state'
import { CellProps } from 'types/table'

interface ExpandableCellProps extends CellProps {
  updateData: (rowIndex: number, columnName: string, value: any, toggle: boolean) => void
}

const ExpandableCell = ({ value, row, column, updateData }: ExpandableCellProps) => {
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
      updateData={updateData}
      expandElement={expandElement}
      whiteSpace={depth > 0}
    />
  )
}

export default ExpandableCell
