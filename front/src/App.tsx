import React, { useEffect, useState } from 'react'
import './App.css'
import Providers from './providers/Providers'
import styled from 'styled-components'
import { EulerAngles, Position, PositionMessage, Target, TargetMessage } from './types/ros';
import { addTargetClient, positionListener, stateListener } from './services/ros';
import expand from './utils'
import ROSLIB from 'roslib';

const Point = ({ name, pose, type }: Target) => {
  return (
    <TableRow>
      <Td>{name}</Td>
      <Td>{type}</Td>
      <Td>{expand(pose.orientation)}</Td>
      <Td>{expand(pose.position)}</Td>
    </TableRow>
  )
}

function App() {
  const [targets, setTargets] = useState<Target[]>([])

  const [position, setPosition] = useState<Position | null>(null)
  const [orientation, setOrientation] = useState<EulerAngles | null>(null)

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
      const positionAsRosMessage = new ROSLIB.Message({...position});

      // TODO: Use proper values.
      const orientationAsRosMessage = new ROSLIB.Message({...orientation});

      const pose = new ROSLIB.Message({
        position: positionAsRosMessage,
        orientation: orientationAsRosMessage
      });

      const request = new ROSLIB.ServiceRequest({
        target: pose,
      });

      addTargetClient.callService(request, (response) => {
        if (!response.success) {
          console.log('ERROR: Failed to add target', pose)
        }
      }, (error) => {
        console.log('ERROR: Failed to add target', pose, ', error:')
        console.error(error)
      })
    }
  }

  return (
    <Providers>
      <Wrapper>
        <Header>test</Header>
        <TargetsContainer>
          <TargetsTable>
            <thead>
              <TableRow>
                <Th>Name</Th>
                <Th>Type</Th>
                <Th>Orientation (alpha, beta, gamma)</Th>
                <Th>Position (x, y, z)</Th>
              </TableRow>
            </thead>
            <tbody>
              {targets.map((target, index) => (
                <Point key={index} {...target} />
              ))}
            </tbody>
          </TargetsTable>
        </TargetsContainer>

        <p>Current position: {expand(position)}</p>
        <p>Current orientation: {expand(orientation)}</p>
        <AddTargetButton onClick={addTarget}>Add current position and orientation as target</AddTargetButton>
      </Wrapper>
    </Providers>
  )
}

const Wrapper = styled.div`
  padding: 0.5rem;
`

const AddTargetButton = styled.button``

const Th = styled.th`
  padding: 0.5rem 1rem;
`
const Td = styled.td`
  padding: 0.5rem 1rem;
`
const TableRow = styled.tr`
  border-bottom: 1px solid #dddddd;

  :nth-of-type(even) {
    background-color: #f3f3f3;
  }

  :last-of-type {
    border-bottom: 2px solid #797979;
  }
`

const TargetsContainer = styled.div`
  overflow-x: auto;
`

const TargetsTable = styled.table`
  //border-spacing: 0.5rem;
  border-collapse: collapse;
  box-shadow: 0 0 20px rgba(0, 0, 0, 0.15);
`

const TargetRow = styled.div`
  display: flex;
  flex-direction: row;
  gap: 0.5rem;
`
const RowElement = styled.div``

const Header = styled.h1`
  color: ${(p) => p.theme.colors.red};
`

export default App
