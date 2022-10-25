import React, {useState} from 'react'
import './App.css'
import Providers from './providers/Providers'
import styled from 'styled-components'
import Targets from 'views/Targets'
import { Route, Routes } from 'react-router-dom'
import Experiment from './views/Experiment'
import DataVisualize from './views/DataVisualize'
import { ExperimentControl } from './components/ExperimentControl'
import { SystemState } from './components/SystemState'
import { Navbar } from './components/Navbar'

const App = () => {

  const [displaySystemControl, setDisplaySystemControl] = useState(true)
  const [displayPlot, setDisplayPlot] = useState(false)
  const [displayTargets, setDisplayTargets] = useState(false)
  const [displayExperiment, setDisplayExperiment] = useState(false)


  return (
    <Providers>
      <Header>mTMS control panel</Header>

      <SmallHeader>System control</SmallHeader>
      <Wrapper>
        <ExperimentControl />
        <SystemState />
      </Wrapper>
      <hr />

      <SmallHeader>Select view</SmallHeader>
      <Navbar />
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
  padding-left: 0.5rem;
`

const SmallHeader = styled.h2`
  color: ${(p) => p.theme.colors.primary};
  padding: 0.5rem;

`

export default App
