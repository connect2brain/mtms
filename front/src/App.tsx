import React from 'react'
import './App.css'
import Providers from './providers/Providers'
import styled from 'styled-components'
import Targets from 'views/Targets'
import { Route, Routes } from 'react-router-dom'
import PulseSequenceConfiguration from './views/PulseSequenceConfiguration'
import Experiment from './views/Experiment'

function App() {
  return (
    <Providers>
      <Header>mTMS control panel</Header>
      <Wrapper>
        <Routes>
          <Route path='/' element={<Experiment />}>
            <Route index element={<Experiment />} />
            <Route path='/targets' element={<Targets />} />
          </Route>
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
