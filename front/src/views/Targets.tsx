import React, { useEffect, useState } from 'react'
import styled from 'styled-components'
import { EulerAngles, Position, PositionMessage, TargetMessage } from 'types/target'
import { addTargetToRos, positionListener, stateListener } from 'services/ros'
import { expand } from 'utils'
import useStore from 'providers/state'
import SequenceTable from 'components/SequenceTable'
import TargetTable from 'components/TargetTable'

const Targets = () => {
  const { targets, setTargets, sequences } = useStore()

  const [position, setPosition] = useState<Position | null>(null)
  const [orientation, setOrientation] = useState<EulerAngles | null>(null)

  const [tab, setTab] = useState<'TARGETS' | 'SEQUENCES'>('TARGETS')

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
    if (position && orientation) {
      addTargetToRos(position, orientation)
    }
  }

  const table = () => {
    switch (tab) {
      case 'TARGETS':
        return <TargetTable />
      case 'SEQUENCES':
        return <SequenceTable />
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
