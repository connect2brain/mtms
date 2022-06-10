import React, { useMemo } from 'react'
import { ChangeableKey } from 'types/target'
import useStore from 'providers/state'
import { MenuItem } from '@szhsin/react-menu'
import '@szhsin/react-menu/dist/index.css'
import { Pulse, PulseSequence } from '../types/pulseSequence'
import Eye from './Eye'
import { EyeCell } from './TableElements/EyeCell'
import EditableCell from './TableElements/EditableCell'
import { GenericTable } from './GenericTable'
import SelectableTargetTableRow from './TableElements/SelectableTargetTableRow'


const TargetTable = () => {
  const { sequences, setSequences, targets } = useStore()

  const columns = useMemo(
    () => [
      {
        Header: () => <Eye visible={true} />,
        accessor: 'visible',
        width: 40,
        Cell: EyeCell,
      },
      {
        Header: 'Name',
        accessor: 'name',
        width: 'auto',
        Cell: EditableCell,
      },
      {
        Header: 'Type',
        accessor: 'type',
        width: 'auto',
      },
      {
        Header: 'Comment',
        accessor: 'comment',
        width: 'auto',
        Cell: EditableCell,
      },
    ],
    [],
  )

  const handleNewSequence = (event: any) => {
    const pulses: Pulse[] = targets
      .filter((t) => t.selected)
      .map((target) => {
        return {
          targetIndex: targets.indexOf(target),
          isi: 100,
          intensity: 0.5,
          modeDuration: 100,
          visible: true,
          selected: false,
        }
      })
    if (pulses.length === 0) {
      console.log('no targets selected for new sequence')
      return
    }

    const newSequence: PulseSequence = {
      pulses,
      name: `sequence-${sequences.length}`,
      selected: false,
      visible: true,
      comment: '',
      iti: 100,
      ibi: 100,
      isis: pulses.map((pulse) => pulse.isi),
      channelInfo: [],
      nofBurstsInTrains: 3,
      nofTrains: 3,
      nofPulsesInBursts: 1,
    }
    setSequences(sequences.concat(newSequence))
    console.log('Created new sequence with targets', pulses.map((t) => targets[t.targetIndex].name).join(', '))
  }

  const filterTargetKeys = () => {
    return targets.map((target) => {
      return {
        name: target.name,
        comment: target.comment,
        type: target.type,
        visible: target.visible,
        selected: target.selected,
      }
    })
  }

  const createMenu = () => {
    return <MenuItem onClick={handleNewSequence}>New sequence from selection</MenuItem>
  }

  return (
    <GenericTable
      columns={columns}
      data={filterTargetKeys()}
      createMenu={createMenu}
      SelectableRow={SelectableTargetTableRow}
    />
  )
}

export default TargetTable
