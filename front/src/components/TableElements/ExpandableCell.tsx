import React, { useEffect, useState } from 'react'
import { useFocusMemo } from '../../utils'
import styled from 'styled-components'
import Expand from '../Expand'
import EditableCell from './EditableCell'
import useStore from 'providers/state'

const ExpandableCell = ({
  value,
  row,
  column,
  updateData, // This is a custom function that we supplied to our table instance,
}: any) => {
  const { canExpand } = row

  const handleClickExpand = (event: any) => {
    event.stopPropagation()
    row.toggleRowExpanded()
  }

  const expandElement = canExpand ? <Expand onClick={handleClickExpand} expanded={row.isExpanded} /> : null

  return <EditableCell value={value} row={row} column={column} updateData={updateData} expandElement={expandElement} />
}

export default ExpandableCell
