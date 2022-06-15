import React, { useMemo } from 'react'
import useStore from 'providers/state'
import { MenuItem, SubMenu } from '@szhsin/react-menu'
import '@szhsin/react-menu/dist/index.css'
import { Pulse, PulseSequence } from '../types/pulseSequence'
import Eye from './Eye'
import { EyeCell } from './TableElements/Cells/EyeCell'
import { EditableTargetTableCell } from './TableElements/Cells/EditableCell'
import { GenericTable } from './GenericTable'
import SelectableTargetTableRow from './TableElements/SelectableTargetTableRow'
import { createPulsesFromSelectedTargets } from '../utils'

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
        Cell: EditableTargetTableCell,
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
        Cell: EditableTargetTableCell,
      },
    ],
    [],
  )

  const handleNewSequence = (event: any) => {
    const pulses: Pulse[] = createPulsesFromSelectedTargets(targets)
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
      intensity: 100,
      isi: 10,
    }
    setSequences(sequences.concat(newSequence))
    console.log('Created new sequence with targets', pulses.map((t) => targets[t.targetIndex].name).join(', '))
  }

  const handleAddToSequence = (sequence: PulseSequence) => {
    const selectedPulses = createPulsesFromSelectedTargets(targets)
    if (selectedPulses.length === 0) {
      console.log('no targets selected for new sequence')
      return
    }

    const pulses = [...sequence.pulses, ...selectedPulses]
    const sequenceIndex = sequences.indexOf(sequence)
    const newSequence = {
      ...sequence,
      pulses,
    }

    const newSequences = sequences.filter((seq) => seq.name !== sequence.name)
    newSequences.splice(sequenceIndex, 0, newSequence)
    setSequences(newSequences)
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
    return (
      <>
        <MenuItem onClick={handleNewSequence}>New sequence from selection</MenuItem>
        <SubMenu label='Add to sequence'>
          {sequences.map((seq) => {
            return (
              <MenuItem onClick={() => handleAddToSequence(seq)} key={seq.name}>
                {seq.name}
              </MenuItem>
            )
          })}
        </SubMenu>
      </>
    )
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
