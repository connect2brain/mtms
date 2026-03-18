import React, { useState } from 'react'
import { Route, Routes } from 'react-router-dom'
import styled from 'styled-components'

import './App.css'

import Providers from './providers/Providers'
import { HealthcheckProvider } from './providers/HealthcheckProvider'
import { ConfigProvider } from './providers/ConfigProvider'
import { SystemProvider } from './providers/SystemProvider'

import { HealthcheckMessageDisplay } from 'components/general/HealthcheckMessageDisplay'
import { HealthcheckStatusDisplay } from 'components/general/HealthcheckStatusDisplay'
import { MultipleViews } from 'views/MultipleViews'

const App = () => {
  return (
    <Providers>
      <HealthcheckStatusDisplay />
      <HealthcheckMessageDisplay />
      <Wrapper>
        <MultipleViews />
      </Wrapper>
    </Providers>
  )
}

const Wrapper = styled.div`
  padding: 1rem;
  margin: 0.5rem;
  background-color: #e8e8e8;
  border-radius: 5px;
  box-shadow: 1px 1px 5px rgba(0, 0, 0, 0.1);
`

export default App
