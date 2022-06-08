import React, { useEffect, useMemo, useState } from 'react'
import styled from 'styled-components'
import { ChangeableKey, EulerAngles, Position, PositionMessage, TargetMessage } from 'types/target'
import { addTargetClient, positionListener, rosServicesByKey, stateListener } from 'services/ros'
import { expand } from 'utils'
import ROSLIB from 'roslib'
import { TargetTable } from 'components/TargetTable'
import Eye from 'components/Eye'
import useStore from '../providers/state'
import { EyeCell } from '../components/TableElements/EyeCell'
import EditableCell from '../components/TableElements/EditableCell'

const Targets = () => {
  const { targets, setTargets, sequences, setSequences } = useStore()

  const [position, setPosition] = useState<Position | null>(null)
  const [orientation, setOrientation] = useState<EulerAngles | null>(null)

  const [tab, setTab] = useState<'TARGETS' | 'SEQUENCES'>('TARGETS')

  const targetTableColumns = useMemo(
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

  const sequenceTableColumns = useMemo(
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

  const updateTargetData = (rowIndex: number, key: ChangeableKey, value: any, toggle: boolean) => {
    const newTargets = [...targets]
    const oldTarget = targets[rowIndex]

    newTargets[rowIndex] = {
      ...oldTarget,
      [key]: value,
    }
    setTargets(newTargets)

    let requestObject = {
      name: oldTarget.name,
    }
    if (!toggle) {
      const requestKey = `new_${key}`
      requestObject = {
        ...requestObject,
        [requestKey]: value,
      }
    }
    const request = new ROSLIB.ServiceRequest(requestObject)
    console.log(request)

    if (!Object.prototype.hasOwnProperty.call(rosServicesByKey, key)) {
      console.error(`Key ${key} is not changeable`)
      return
    }
    rosServicesByKey[key].callService(
      request,
      (result) => {
        if (!result.success) {
          console.error(`ERROR: Failed to change key '${key}' from ${oldTarget[key]} to ${value}`)
        } else {
          console.log(`Changed ${oldTarget.name} key '${key}' from ${oldTarget[key]} to ${value}`)
        }
      },
      (error) => {
        console.error(error)
      },
    )
  }

  const updateTargets = (message: TargetMessage) => {
    setTargets(message.targets)
  }

  const updatePosition = (message: PositionMessage) => {
    setPosition(message.position)
    setOrientation(message.orientation)
  }

  useEffect(() => {
    stateListener.subscribe(updateTargets)
    positionListener.subscribe(updatePosition)
  }, [])

  const addTarget = () => {
    if (position) {
      const positionAsRosMessage = new ROSLIB.Message(position)

      const orientationAsRosMessage = new ROSLIB.Message(orientation)

      const pose = new ROSLIB.Message({
        position: positionAsRosMessage,
        orientation: orientationAsRosMessage,
      })

      const request = new ROSLIB.ServiceRequest({
        target: pose,
      })

      addTargetClient.callService(
        request,
        (response) => {
          if (!response.success) {
            console.log('ERROR: Failed to add target', pose)
          }
        },
        (error) => {
          console.log('ERROR: Failed to add target', pose, ', error:')
          console.error(error)
        },
      )
    }
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

  const filterSequenceKeys = () => {
    return sequences.map((seq) => {
      return {
        seqName: seq.name,
        seqComment: seq.comment,
        seqIntensity: 0,
        seqVisible: seq.visible,
        seqSelected: seq.selected,
      }
    })
  }

  const table = () => {
    switch (tab) {
      case 'TARGETS':
        return <TargetTable columns={targetTableColumns} data={filterTargetKeys()} updateData={updateTargetData} />
      case 'SEQUENCES':
        return <TargetTable columns={sequenceTableColumns} data={filterSequenceKeys()} updateData={updateTargetData} />
    }
  }

  const handleViewChangeClick = (event: any) => {
    const target = event.target.name
    setTab(target)
  }

  return (
    <>
      <button name='TARGETS' onClick={handleViewChangeClick}>
        targets
      </button>
      <button name='SEQUENCES' onClick={handleViewChangeClick}>
        sequences
      </button>

      <br />

      {table()}

      <p>Current position: {expand(position)}</p>
      <p>Current orientation: {expand(orientation)}</p>
      <AddTargetButton onClick={addTarget}>Add current position and orientation as target</AddTargetButton>
    </>
  )
}

const AddTargetButton = styled.button``

export default Targets
