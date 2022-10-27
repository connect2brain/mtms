import React, { useState } from 'react'
import styled from 'styled-components'
import { ExperimentControl } from '../components/ExperimentControl'
import { SystemState } from '../components/SystemState'
import Targets from './Targets'
import DataVisualize from './DataVisualize'
import { SmallHeader } from '../styles/StyledTypography'

export const MultipleViews = () => {
  const [displaySystemControl, setDisplaySystemControl] = useState(true)
  const [displayPlot, setDisplayPlot] = useState(false)
  const [displayWebGLPlot, setDisplayWebGLPlot] = useState(false)
  const [displayTargets, setDisplayTargets] = useState(false)
  const [displayExperiment, setDisplayExperiment] = useState(false)

  const toggleView = (enabled: boolean, toggleFunction: React.Dispatch<React.SetStateAction<boolean>>) => {
    toggleFunction(!enabled)
  }

  return (
    <div>
      <OptionWrapper>
        <input
          id='toggle-system-control'
          type='checkbox'
          checked={displaySystemControl}
          onChange={() => toggleView(displaySystemControl, setDisplaySystemControl)}
        />
        <label htmlFor='toggle-system-control'>System Control</label>
        <br />

        <input
          id='toggle-plot'
          type='checkbox'
          checked={displayPlot}
          onChange={() => toggleView(displayPlot, setDisplayPlot)}
        />
        <label htmlFor='toggle-plot'>EEG plot</label>
        <br />

        <input
          id='toggle-webgl-plot'
          type='checkbox'
          checked={displayWebGLPlot}
          onChange={() => toggleView(displayWebGLPlot, setDisplayWebGLPlot)}
        />
        <label htmlFor='toggle-webgl-plot'>EEG WebGL plot</label>
        <br />

        <input
          id='toggle-targets'
          type='checkbox'
          checked={displayTargets}
          onChange={() => toggleView(displayTargets, setDisplayTargets)}
        />
        <label htmlFor='toggle-targets'>Targets table</label>
        <br />
      </OptionWrapper>
      <ViewContainer>
        {displaySystemControl ? (
          <Wrapper>
            <SmallHeader>System control</SmallHeader>
            <ExperimentControl />
            <SystemState />
          </Wrapper>
        ) : null}

        {displayPlot ? (
          <Wrapper>
            <SmallHeader>EEG plot</SmallHeader>
            <DataVisualize webgl={false}/>
          </Wrapper>
        ) : null}

        {displayWebGLPlot ? (
          <Wrapper>
            <SmallHeader>EEG WebGL plot</SmallHeader>
            <DataVisualize webgl={true}/>
          </Wrapper>
        ) : null}

        {displayTargets ? (
          <Wrapper>
            <SmallHeader>Targets table</SmallHeader>
            <Targets />
          </Wrapper>
        ) : null}
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
  width: 48%;
  padding: 0.5rem;
  margin: 0.5rem;
  border: 1px solid ${(p) => p.theme.colors.darkgray};
`
