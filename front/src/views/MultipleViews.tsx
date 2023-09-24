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
        <a href="#" onClick={() => setCurrentView('systemControl')}>System Control</a><br />
        <a href="#" onClick={() => setCurrentView('plot')}>EEG plot</a><br />
        <a href="#" onClick={() => setCurrentView('webGLPlot')}>EEG WebGL plot</a><br />
        <a href="#" onClick={() => setCurrentView('targets')}>Targets table</a><br />
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
            <SmallHeader>EEG plot</SmallHeader>
            <DataVisualize />
          </Wrapper>
        )}

        {currentView === 'webGLPlot' && (
          <Wrapper>
            <SmallHeader>EEG WebGL plot</SmallHeader>
            <DataVisualizeWebGL />
          </Wrapper>
        )}

        {currentView === 'targets' && (
          <Wrapper>
            <SmallHeader>Targets table</SmallHeader>
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
`

const Wrapper = styled.div`
  width: 100%;
  padding: 0.5rem;
  margin: 0.5rem;
  border: 1px solid ${(p) => p.theme.colors.darkgray};
`
