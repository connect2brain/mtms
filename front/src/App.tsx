import React, { useEffect, useState } from 'react'
import './App.css'
import Providers from './providers/Providers'
import styled from 'styled-components'
import { Target, TargetMessage } from './types/ros'
import { stateListener } from './services/ros'

const Point = ({ name, pose, type }: Target) => {

  const expand = (obj: any) =>
    Object.keys(obj)
      .map((key) => obj[key].toFixed(3))
      .join(', ')

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

  const updateTargets = (message: TargetMessage) => {
    setTargets(message.targets)
  }

  useEffect(() => {
    stateListener.subscribe(updateTargets)
  }, [])

  return (
    <Providers>
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
    </Providers>
  )
}

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
  margin: 20px;
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
