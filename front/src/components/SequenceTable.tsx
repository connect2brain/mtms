import React, { useMemo } from 'react'

import { ChangeableKey } from 'types/target'
import useStore from 'providers/state'
import { MenuItem } from '@szhsin/react-menu'
import '@szhsin/react-menu/dist/index.css'
import Eye from './Eye'
import { EyeCell } from './TableElements/EyeCell'
import EditableCell from './TableElements/EditableCell'
import { GenericTable } from './GenericTable'
import ExpandableCell from './TableElements/ExpandableCell'
import SelectableSequenceTableRow from './TableElements/SelectableSequenceTableRow'

interface TableProps {
  updateData: (rowIndex: number, key: ChangeableKey, value: any, toggle: boolean) => void
}

const SequenceTable = ({ updateData }: TableProps) => {
  const { sequences, targets } = useStore()

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
        const target = targets[pulse.targetIndex]
        return {
          seqName: target.name,
          seqComment: target.comment,
          seqIntensity: 0,
          seqVisible: pulse.visible,
          selected: pulse.selected,
          seqIsi: pulse.isi,
        }
      })

      return {
        seqName: seq.name,
        seqComment: seq.comment,
        seqIntensity: 0,
        seqVisible: seq.visible,
        selected: seq.selected,
        subRows,
        seqIsi: 0,
      }
    })
  }
  const createMenu = () => {
    return <MenuItem>moi</MenuItem>
  }

  return (
    <GenericTable
      columns={columns}
      data={filterSequenceKeys()}
      updateData={updateData}
      createMenu={createMenu}
      SelectableRow={SelectableSequenceTableRow}
    />
  )
}

export default SequenceTable
