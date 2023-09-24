import React, { useState } from 'react'
import './App.css'
import Providers from './providers/Providers'
import styled from 'styled-components'
import Targets from 'views/Targets'
import { Route, Routes } from 'react-router-dom'
import Session from './views/Session'
import DataVisualize from './views/DataVisualize'
import { MultipleViews } from './views/MultipleViews'
import { Header as StyledHeader } from './styles/StyledTypography'

const App = () => {
  return (
    <Providers>
      <Header>mTMS control panel</Header>
      <Wrapper>
        <Routes>
          <Route path='/' element={<MultipleViews />} />
          <Route path='/targets' element={<Targets />} />
          <Route path='/session' element={<Session />} />
          <Route path='/plot' element={<DataVisualize />} />
        </Routes>
      </Wrapper>
    </Providers>
  )
}

const Header = styled(StyledHeader)`
  font-size: 1.8rem;
  color: #333;
  margin-bottom: 1rem;
  padding: 0.5rem;
  background-color: #f2f2f2;
  border-bottom: 2px solid #ddd;
  border-radius: 3px 3px 0 0;
`

const Wrapper = styled.div`
  padding: 1rem;
  margin: 0.5rem;
  background-color: #e8e8e8;
  border-radius: 5px;
  box-shadow: 1px 1px 5px rgba(0, 0, 0, 0.1);
`

export default App
