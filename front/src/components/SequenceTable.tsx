import React, { useMemo, useState } from 'react'

import { ChangeableKey } from 'types/target'
import useStore from 'providers/state'
import { ControlledMenu, MenuItem, useMenuState } from '@szhsin/react-menu'
import '@szhsin/react-menu/dist/index.css'
import Eye from './Eye'
import { EyeCell } from './TableElements/EyeCell'
import EditableCell from './TableElements/EditableCell'
import { GenericTable } from './GenericTable'
import ExpandableCell from './TableElements/ExpandableCell'

interface TableProps {
  updateData: (rowIndex: number, key: ChangeableKey, value: any, toggle: boolean) => void
}

const SequenceTable = ({ updateData }: TableProps) => {
  const { sequences } = useStore()

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
        Cell: ExpandableCell,
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
  const filterSequenceKeys = () => {
    return sequences.map((seq) => {
      const subRows = seq.pulses.map((pulse) => {
        return {
          seqName: pulse.target.name,
          seqComment: pulse.target.comment,
          seqIntensity: 0,
          seqVisible: pulse.target.visible,
          seqSelected: pulse.target.selected,
        }
      })

      return {
        seqName: seq.name,
        seqComment: seq.comment,
        seqIntensity: 0,
        seqVisible: seq.visible,
        seqSelected: seq.selected,
        subRows,
      }
    })
  }
  const createMenu = () => {
    return <MenuItem>moi</MenuItem>
  }

  return <GenericTable columns={columns} data={filterSequenceKeys()} updateData={updateData} createMenu={createMenu} />
}

export default SequenceTable
