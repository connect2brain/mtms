import React, { useMemo, useState } from 'react'

import { useTable, Row, useExpanded } from 'react-table'
import styled from 'styled-components'
import { ChangeableKey } from 'types/target'
import useStore from 'providers/state'
import { ControlledMenu, MenuItem, useMenuState } from '@szhsin/react-menu'
import '@szhsin/react-menu/dist/index.css'
import { Pulse, PulseSequence } from '../types/pulseSequence'
import NotEditableCell from './TableElements/NotEditableCell'
import SelectableTableRow from './TableElements/SelectableTableRow'
import Eye from './Eye'
import { EyeCell } from './TableElements/EyeCell'
import EditableCell from './TableElements/EditableCell'
import { GenericTable } from './GenericTable'

interface TableProps {
  data: any[]
  updateData: (rowIndex: number, key: ChangeableKey, value: any, toggle: boolean) => void
}

const SequenceTable = ({ data, updateData }: TableProps) => {
  const columns = useMemo(
    () => [
      {
        Header: () => <Eye visible={true} />,
        accessor: 'seqVisible',
        width: 40,
        Cell: EyeCell,
      },
      {
        Header: 'Name',
        accessor: 'seqName',
        width: 'auto',
        Cell: EditableCell,
      },
      {
        Header: 'Intensity',
        accessor: 'seqIntensity',
        width: 'auto',
        Cell: EditableCell,
      },
      {
        Header: 'isi',
        accessor: 'seqIsi',
        width: 'auto',
        Cell: EditableCell,
      },
      {
        Header: 'Mode Duration',
        accessor: 'seqModeDuration',
        width: 'auto',
        Cell: EditableCell,
      },
    ],
    [],
  )

  const createMenu = () => {
    return <MenuItem>moi</MenuItem>
  }

  return <GenericTable columns={columns} data={data} updateData={updateData} createMenu={createMenu} />
}

export default SequenceTable
