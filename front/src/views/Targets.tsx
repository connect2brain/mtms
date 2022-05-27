import React, { useEffect, useMemo, useState } from 'react'
import styled from 'styled-components'
import { ChangeableKey, EulerAngles, Position, PositionMessage, Target, TargetMessage } from 'types/target'
import { addTargetClient, positionListener, rosServicesByKey, stateListener } from 'services/ros'
import { expand } from 'utils'
import ROSLIB from 'roslib'
import { EditableCell, TargetTable } from 'components/TargetTable'

const Targets = () => {
  const [targets, setTargets] = useState<Target[]>([])

  const [position, setPosition] = useState<Position | null>(null)
  const [orientation, setOrientation] = useState<EulerAngles | null>(null)
  const [skipPageReset, setSkipPageReset] = useState(false)

  const targetTableColumns = useMemo(
    () => [
      {
        Header: 'Targets',
        columns: [
          {
            Header: 'Name',
            accessor: 'name',
            Cell: EditableCell,
          },
          {
            Header: 'Orientation (alpha, beta, gamma)',
            accessor: 'orientation',
          },
          {
            Header: 'Position (x, y, z)',
            accessor: 'position',
          },
          {
            Header: 'Comment',
            accessor: 'comment',
            Cell: EditableCell,
          },
        ],
      },
    ],
    [],
  )

  const updateTargetData = (rowIndex: number, key: ChangeableKey, value: any) => {
    setSkipPageReset(true)

    const newTargets = [...targets]
    const oldTarget = targets[rowIndex]

    newTargets[rowIndex] = {
      ...oldTarget,
      [key]: value,
    }
    setTargets(newTargets)

    const requestKey = `new_${key}`
    const request = new ROSLIB.ServiceRequest({
      name: oldTarget.name,
      [requestKey]: value,
    })

    if (!Object.prototype.hasOwnProperty.call(rosServicesByKey, key)) {
      console.error(`Key ${key} is not changeable`)
      return
    }
    rosServicesByKey[key].callService(
      request,
      (result) => {
        if (!result.success) {
          console.error(`ERROR: Failed to change key ${key} from ${oldTarget[key]} to ${value}`)
        } else {
          console.log(`Changed ${oldTarget.name} key ${key} from ${oldTarget[key]} to ${value}`)
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

  useEffect(() => {
    setSkipPageReset(false)
  }, [targets])

  const addTarget = () => {
    if (position) {
      const positionAsRosMessage = new ROSLIB.Message({ ...position })

      const orientationAsRosMessage = new ROSLIB.Message({ ...orientation })

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
        orientation: expand(target.pose.orientation),
        position: expand(target.pose.position),
        comment: target.comment,
      }
    })
  }

  return (
    <>
      <TargetTable
        columns={targetTableColumns}
        data={filterTargetKeys()}
        updateData={updateTargetData}
        skipPageReset={skipPageReset}
      />
      <br />

      <p>Current position: {expand(position)}</p>
      <p>Current orientation: {expand(orientation)}</p>
      <AddTargetButton onClick={addTarget}>Add current position and orientation as target</AddTargetButton>
    </>
  )
}

const AddTargetButton = styled.button``

export default Targets
