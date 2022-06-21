import React, { useMemo } from 'react'
import { MenuItem, SubMenu } from '@szhsin/react-menu'
import '@szhsin/react-menu/dist/index.css'
import { Pulse, PulseSequence } from '../types/pulseSequence'
import Eye from './Eye'
import { EyeCell } from './TableElements/Cells/EyeCell'
import { EditableTargetTableCell } from './TableElements/Cells/EditableCell'
import { GenericTable } from './GenericTable'
import SelectableTargetTableRow from './TableElements/SelectableTargetTableRow'
import { createPulsesFromSelectedTargets } from 'utils'
import { useAppSelector } from 'providers/reduxHooks'
import { addSequence, modifySequence, removePulsesFromSequence } from 'reducers/sequenceReducer'
import { useDispatch } from 'react-redux'
import { addPulseSequenceToRos, removeTargetInRos } from 'services/ros'

const TargetTable = () => {
  const { sequences } = useAppSelector((state) => state.sequences)
  const { targets } = useAppSelector((state) => state.targets)

  const dispatch = useDispatch()

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

    //dispatch(addSequence(newSequence))
    addPulseSequenceToRos(pulses)
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
    dispatch(
      modifySequence({
        index: sequenceIndex,
        pulseSequence: newSequence,
      }),
    )
  }

  const handleRemoveTargets = () => {
    targets
      .filter((target) => target.selected)
      .forEach((target) => {
        const targetIndex = targets.indexOf(target)
        removeTargetInRos(target)

        sequences.forEach((sequence, sequenceIndex) => {
          const pulses = sequence.pulses.filter((pulse) => pulse.targetIndex === targetIndex)
          dispatch(
            removePulsesFromSequence({
              sequenceIndex,
              pulseIdsToRemove: pulses.map((pulse, id) => id),
            }),
          )
        })
      })
  }

  const createMenu = () => {
    return (
      <>
        <MenuItem onClick={handleNewSequence} id='create-new-sequence'>New sequence from selection</MenuItem>
        <MenuItem onClick={handleRemoveTargets}>Remove selected target(s)</MenuItem>
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
