import React, { useState, useEffect } from 'react'
import styled from 'styled-components'

import DataVisualize from './DataVisualize'
import DataVisualizeWebGL from './DataVisualizeWebGL'

import Targets from './Targets'
import { SmallHeader } from '../styles/StyledTypography'

import { SystemView } from './SystemView'
import { ExperimentView } from './ExperimentView'
import { PipelineView } from './PipelineView'

/* Session storage utilities. */

const storeKey = (key: string, value: any) => {
  sessionStorage.setItem(key, JSON.stringify(value))
}

const getKey = (key: string, defaultValue: any): any => {
  const storedValue = sessionStorage.getItem(key)
  return storedValue !== null ? JSON.parse(storedValue) : defaultValue
}

export const MultipleViews = () => {
  const [currentView, setCurrentView] = useState(() => getKey('currentView', 'SystemView'))

  useEffect(() => {
    storeKey('currentView', currentView)
  }, [currentView])

  return (
    <div>
      <OptionWrapper>
        <a
          href="#"
          onClick={() => setCurrentView('SystemView')}
          className={currentView === 'SystemView' ? 'active' : ''}
        >
          System
        </a>
        <a
          href="#"
          onClick={() => setCurrentView('experiment')}
          className={currentView === 'experiment' ? 'active' : ''}
        >
          Experiment
        </a>
        <a
          href="#"
          onClick={() => setCurrentView('pipeline')}
          className={currentView === 'pipeline' ? 'active' : ''}
        >
          Pipeline
        </a>
        <a
          href="#"
          onClick={() => setCurrentView('plot')}
          className={currentView === 'plot' ? 'active' : ''}
        >
          EEG
        </a>
        <a
          href="#"
          onClick={() => setCurrentView('webGLPlot')}
          className={currentView === 'webGLPlot' ? 'active' : ''}
        >
          EEG (WebGL)
        </a>
        <a
          href="#"
          onClick={() => setCurrentView('targets')}
          className={currentView === 'targets' ? 'active' : ''}
        >
          Targeting
        </a>
      </OptionWrapper>
      <ViewContainer>
        <Wrapper style={{ display: currentView === 'SystemView' ? 'block' : 'none' }}>
          <SmallHeader>System</SmallHeader>
          <SystemView />
        </Wrapper>
        <Wrapper style={{ display: currentView === 'experiment' ? 'block' : 'none' }}>
          <SmallHeader>Experiment</SmallHeader>
          <ExperimentView />
        </Wrapper>
        <Wrapper style={{ display: currentView === 'pipeline' ? 'block' : 'none' }}>
          <SmallHeader>Pipeline</SmallHeader>
          <PipelineView />
        </Wrapper>
        <Wrapper style={{ display: currentView === 'plot' ? 'block' : 'none' }}>
          <SmallHeader>EEG</SmallHeader>
          <DataVisualize />
        </Wrapper>
        <Wrapper style={{ display: currentView === 'webGLPlot' ? 'block' : 'none' }}>
          <SmallHeader>EEG (WebGL)</SmallHeader>
          <DataVisualizeWebGL />
        </Wrapper>
        <Wrapper style={{ display: currentView === 'targets' ? 'block' : 'none' }}>
          <SmallHeader>Targeting</SmallHeader>
          <Targets />
        </Wrapper>
      </ViewContainer>
    </div>
  )
}

const ViewContainer = styled.div`
  display: flex;
  flex-wrap: wrap;
`

const OptionWrapper = styled.div`
  margin: 0.5rem;

  a {
    text-decoration: none;
    color: #505050;   // Darker gray for regular links
    padding: 0.5rem;
    display: inline-block;
    transition: color 0.3s ease;

    &:hover {
      color: #303030;  // Even darker gray for hover
    }

    &.active {
      color: #222222;  // Almost black for active link
      font-weight: bold;
    }
  }
`

const Wrapper = styled.div`
  width: 100%;
  padding: 0.5rem;
  margin: 0.5rem;
  border: 3px solid ${(p) => p.theme.colors.darkgray};
`
