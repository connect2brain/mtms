import React from 'react'
import styled from 'styled-components'

import './App.css'

import Providers from './providers/Providers'

import { HealthcheckMessageDisplay } from 'components/general/HealthcheckMessageDisplay'
import { HealthcheckStatusDisplay } from 'components/general/HealthcheckStatusDisplay'
import { EegInfoDisplay } from 'components/general/EegInfoDisplay'
import { ChannelVoltageDisplay } from 'components/general/ChannelVoltageDisplay'
import { RosConnectionOverlay } from 'components/general/RosConnectionOverlay'
import { MultipleViews } from 'views/MultipleViews'

const App = () => {
  return (
    <Providers>
      <HealthcheckStatusDisplay />
      <HealthcheckMessageDisplay />
      <EegInfoDisplay />
      <ChannelVoltageDisplay />
      <Wrapper>
        <MultipleViews />
      </Wrapper>
      <RosConnectionOverlay />
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
