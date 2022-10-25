import React, { useState } from 'react'
import './App.css'
import Providers from './providers/Providers'
import styled from 'styled-components'
import Targets from 'views/Targets'
import { Route, Routes } from 'react-router-dom'
import Experiment from './views/Experiment'
import DataVisualize from './views/DataVisualize'
import { MultipleViews } from './views/MultipleViews'
import { Header, SmallHeader } from './styles/StyledTypography'

const App = () => {
  return (
    <Providers>
      <Header>mTMS control panel</Header>

      <hr />

      <Wrapper>
        <Routes>
          <Route path='/' element={<MultipleViews />} />
          <Route path='/targets' element={<Targets />} />
          <Route path='/experiment' element={<Experiment />} />
          <Route path='/plot' element={<DataVisualize />} />
        </Routes>
      </Wrapper>
    </Providers>
  )
}

const Wrapper = styled.div`
  padding: 0.5rem;
  margin: 0.5rem;
`

export default App
