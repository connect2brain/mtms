import React from 'react'
import './App.css'
import Providers from './providers/Providers'
import styled from 'styled-components'
import Targets from 'views/Targets'
import { Route, Routes } from 'react-router-dom'
import Experiment from './views/Experiment'
import DataVisualize from './views/DataVisualize'
import { ExperimentControl } from './components/ExperimentControl'
import { SystemState } from './components/SystemState'

function App() {
  return (
    <Providers>
      <Header>mTMS control panel</Header>

      <Wrapper>
        <ExperimentControl />
        <SystemState />
      </Wrapper>
      <hr />

      <Wrapper>
        <Routes>
          <Route path='/' element={<Targets />} />
          <Route path='/experiment' element={<Experiment />} />
          <Route path='/plot' element={<DataVisualize />} />
        </Routes>
      </Wrapper>
    </Providers>
  )
}

const Wrapper = styled.div`
  padding: 0.5rem;
`

const Header = styled.h1`
  color: ${(p) => p.theme.colors.primary};
`

export default App
