import React, { useState, useEffect } from 'react'
import styled from 'styled-components'

import { SmallHeader } from '../styles/StyledTypography'

import { ConfigView } from './ConfigView'
import { SystemView } from './SystemView'
import { ExperimentView } from './ExperimentView'

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
          href='#'
          onClick={() => setCurrentView('SystemView')}
          className={currentView === 'SystemView' ? 'active' : ''}
        >
          System
        </a>
        <a
          href='#'
          onClick={() => setCurrentView('experiment')}
          className={currentView === 'experiment' ? 'active' : ''}
        >
          Experiment
        </a>
        <a
          href='#'
          onClick={() => setCurrentView('config')}
          className={currentView === 'config' ? 'active' : ''}
        >
          Config
        </a>
      </OptionWrapper>
      <ViewContainer>
        {currentView === 'SystemView' && (
        <Wrapper>
          <SmallHeader>System</SmallHeader>
          <SystemView />
        </Wrapper>
        )}
        {currentView === 'experiment' && (
        <Wrapper>
          <SmallHeader>Experiment</SmallHeader>
          <ExperimentView />
        </Wrapper>
        )}
        {currentView === 'config' && (
        <Wrapper>
          <SmallHeader>Config</SmallHeader>
          <ConfigView />
        </Wrapper>
        )}
      </ViewContainer>
    </div>
  )
}

const ViewContainer = styled.div`
  display: flex;
  flex-wrap: wrap;
  padding-top: 0.75rem;
  position: relative;

  &::before {
    content: '';
    position: absolute;
    top: 0;
    left: 0.5rem;
    width: 250px;
    border-top: 2px solid ${(p) => p.theme.colors.gray};
  }
`

const OptionWrapper = styled.div`
  margin: 0.5rem;

  a {
    text-decoration: none;
    color: #505050; // Darker gray for regular links
    padding: 0.5rem;
    display: inline-block;
    transition: color 0.3s ease;

    &:hover {
      color: #303030; // Even darker gray for hover
    }

    &.active {
      color: #222222; // Almost black for active link
      font-weight: bold;
    }
  }
`

const Wrapper = styled.div`
  width: 100%;
  padding: 0.5rem;
  margin: 0.5rem;
`

