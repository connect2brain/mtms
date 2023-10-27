import React, { useState } from 'react'
import { Route, Routes } from 'react-router-dom'
import styled from 'styled-components'

import './App.css'

import Providers from './providers/Providers'
import { HealthcheckProvider } from './providers/HealthcheckProvider'
import { PipelineProvider } from './providers/PipelineProvider'
import { EegProvider } from './providers/EegProvider'
import { SystemProvider } from './providers/SystemProvider'

import { HealthcheckStatusDisplay } from 'components/HealthcheckStatusDisplay'
import { MultipleViews } from 'views/MultipleViews'
import { Header as StyledHeader } from 'styles/StyledTypography'
import { StyledPanel } from 'styles/General'
import { ProjectProvider } from 'providers/ProjectProvider'

const App = () => {
  return (
    <Providers>
      <ProjectProvider>
        <SystemProvider>
          <PipelineProvider>
            <EegProvider>
              <HealthcheckProvider>
                <Header>mTMS control panel</Header>
                <HealthcheckPanel>
                  <HealthcheckStatusDisplay />
                </HealthcheckPanel>
                <Wrapper>
                  <MultipleViews />
                </Wrapper>
              </HealthcheckProvider>
            </EegProvider>
          </PipelineProvider>
        </SystemProvider>
      </ProjectProvider>
    </Providers>
  )
}

const HealthcheckPanel = styled.div`
  width: 155px;
  height: 40px;
  position: fixed;
  top: 0;
  right: 0;
  z-index: 1000;
  ${StyledPanel}
`

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
