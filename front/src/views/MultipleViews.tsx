import React, { useState } from 'react'
import styled from 'styled-components'
import { SessionControl } from '../components/SessionControl'
import { SystemState } from '../components/SystemState'
import Targets from './Targets'
import DataVisualize from './DataVisualize'
import { SmallHeader } from '../styles/StyledTypography'
import DataVisualizeWebGL from './DataVisualizeWebGL'
import { SystemControl } from './SystemControl'

export const MultipleViews = () => {
  const [currentView, setCurrentView] = useState('systemControl')

  return (
    <div>
      <OptionWrapper>
        <a
          href="#"
          onClick={() => setCurrentView('systemControl')}
          className={currentView === 'systemControl' ? 'active' : ''}
        >
          System Control
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
        {currentView === 'systemControl' && (
          <Wrapper>
            <SmallHeader>System control</SmallHeader>
            <SystemControl />
          </Wrapper>
        )}

        {currentView === 'plot' && (
          <Wrapper>
            <SmallHeader>EEG</SmallHeader>
            <DataVisualize />
          </Wrapper>
        )}

        {currentView === 'webGLPlot' && (
          <Wrapper>
            <SmallHeader>EEG (WebGL)</SmallHeader>
            <DataVisualizeWebGL />
          </Wrapper>
        )}

        {currentView === 'targets' && (
          <Wrapper>
            <SmallHeader>Targeting</SmallHeader>
            <Targets />
          </Wrapper>
        )}
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
  border: 1px solid ${(p) => p.theme.colors.darkgray};
`
