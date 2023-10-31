import React, { useContext, useEffect, useState, useRef } from 'react'
import styled from 'styled-components'

import { TabBar } from 'styles/General'

import { EegDisplay } from 'components/EegDisplay'
import { LatencyDisplay } from 'components/LatencyDisplay'
import { SessionDisplay } from 'components/SessionDisplay'

import { SmallerTitle } from 'styles/ExperimentStyles'
import { StyledPanel, ProjectRow, ConfigRow, ConfigLabel, Select } from 'styles/General'

import { ToggleSwitch } from 'components/Experiment/ToggleSwitch'

import {
  listProjects,
  setActiveProject,
  setPreprocessorRos,
  setPreprocessorEnabledRos,
  setDeciderRos,
  setDeciderEnabledRos,
} from 'ros/ros'

import { PipelineContext } from 'providers/PipelineProvider'
import { ProjectContext } from 'providers/ProjectProvider'

const InputRow = styled.div`
  display: flex;
  justify-content: flex-start;
  align-items: center;
  gap: 10px;
  margin-bottom: 30px;
`

const Label = styled.label`
  width: 150px;
  text-align: left;
  margin-right: 10px;
  margin-left: 30px;
  display: inline-block;
`

/* Pipeline definition */
const PipelinePanel = styled.div`
  display: grid;
  grid-template-rows: repeat(2, 1fr);
  grid-template-columns: repeat(4, 1fr);
  width: 600px;
  height: 450px;
  gap: 48px;
  position: relative;
  margin-left: 30px;
`

const EegCircle = styled.div`
  display: flex;
  justify-content: center;
  align-items: center;
  grid-row: 1 / 2;
  grid-column: 1 / 2;
  width: 70px;
  height: 70px;
  margin-top: 74px;
  background-color: #d46c0b;
  border-radius: 50%;
  font-weight: bold;
`

const PreprocessorPanel = styled(StyledPanel)`
  grid-row: 1 / 2;
  grid-column: 2 / 3;
  width: 250px;
  height: 150px;
`

const DeciderPanel = styled(StyledPanel)`
  grid-row: 1 / 2;
  grid-column: 3 / 4;
  width: 250px;
  height: 150px;
`

const PresenterPanel = styled(StyledPanel)`
  grid-row: 1 / 2;
  grid-column: 4 / 5;
  width: 250px;
  height: 150px;
`

const TmsPanel = styled(StyledPanel)`
  grid-row: 2 / 3;
  grid-column: 4 / 5;
  width: 250px;
  height: 90px;
`

const Arrow = styled.div`
  width: 30px; /* Arrow width */
  height: 8px; /* Arrow thickness */
  background: #888;
  position: absolute;
  transform: translateY(-50%);

  &:after {
    content: '';
    position: absolute;
    top: 50%;
    right: -9px;
    transform: translateY(-50%);
    border-top: 10px solid transparent;
    border-bottom: 10px solid transparent;
    border-left: 10px solid #888;
  }
`

/* Session storage utilities. */

const getData = (): any => {
  const data = sessionStorage.getItem('pipeline')
  return data ? JSON.parse(data) : {}
}

const storeKey = (key: string, value: any) => {
  const currentData = getData()
  currentData[key] = value
  sessionStorage.setItem('pipeline', JSON.stringify(currentData))
}

const getKey = (key: string, defaultValue: any): any => {
  const data = getData()
  return key in data ? data[key] : defaultValue
}

export const PipelineView = () => {
  const { activeProject } = useContext(ProjectContext)

  const { preprocessorList, preprocessorEnabled, deciderList, deciderEnabled } = useContext(PipelineContext)

  const [projects, setProjects] = useState<string[]>([])

  const [preprocessor, setPreprocessor] = useState<string>('')
  const [decider, setDecider] = useState<string>('')

  const [presenterEnabled, setPresenterEnabled] = useState<boolean>(() => getKey('presenterEnabled', false))

  const handleProjectChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    const newActiveProject = event.target.value
    setActiveProject(newActiveProject, () => {
      console.log('Active project set to ' + newActiveProject)
    })
  }

  /* Preprocessor */
  const handlePreprocessorEnabled = (enabled: boolean) => {
    setPreprocessorEnabledRos(enabled, () => {
      console.log('Preprocessor ' + (enabled ? 'enabled' : 'disabled'))
    })
  }

  const handlePreprocessorChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    const preprocessor = event.target.value
    setPreprocessor(preprocessor)

    setPreprocessorRos(preprocessor, () => {
      console.log('Preprocessor set to ' + preprocessor)
    })
  }

  /* Decider */
  const handleDeciderEnabled = (enabled: boolean) => {
    setDeciderEnabledRos(enabled, () => {
      console.log('Decider ' + (enabled ? 'enabled' : 'disabled'))
    })
  }

  const handleDeciderChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    const decider = event.target.value
    setDecider(decider)

    setDeciderRos(decider, () => {
      console.log('Decider set to ' + decider)
    })
  }

  /* Set list of projects. */
  useEffect(() => {
    listProjects((projects) => {
      setProjects(projects)
    })
  }, [])

  /* Update session storage. */
  useEffect(() => {
    storeKey('preprocessorEnabled', preprocessorEnabled)
  }, [preprocessorEnabled])

  useEffect(() => {
    storeKey('deciderEnabled', deciderEnabled)
  }, [deciderEnabled])

  useEffect(() => {
    storeKey('presenterEnabled', presenterEnabled)
  }, [presenterEnabled])

  return (
    <>
      <ProjectRow>
        <Label>Project:</Label>
        <Select onChange={handleProjectChange} value={activeProject}>
          {projects.map((project, index) => (
            <option key={index} value={project}>
              {project}
            </option>
          ))}
        </Select>
      </ProjectRow>

      <PipelinePanel>
        <Arrow style={{ left: '75px', top: '110px' }} />
        <Arrow style={{ left: '408px', top: '110px' }} />
        <Arrow style={{ left: '741px', top: '110px' }} />
        <Arrow style={{ left: '734px', top: '234px', width: '44px', transform: 'rotate(45deg)' }} />
        <EegCircle>EEG</EegCircle>
        <PreprocessorPanel>
          <SmallerTitle>Preprocessor</SmallerTitle>
          <ConfigRow>
            <ConfigLabel>Enabled:</ConfigLabel>
            <ToggleSwitch type='flat' checked={preprocessorEnabled} onChange={handlePreprocessorEnabled} />
          </ConfigRow>
          <ConfigRow>
            <ConfigLabel>Module:</ConfigLabel>
            <Select onChange={handlePreprocessorChange} value={preprocessor}>
              {preprocessorList.map((preprocessor, index) => (
                <option key={index} value={preprocessor}>
                  {preprocessor}
                </option>
              ))}
            </Select>
          </ConfigRow>
        </PreprocessorPanel>
        <DeciderPanel>
          <SmallerTitle>Decider</SmallerTitle>
          <ConfigRow>
            <ConfigLabel>Enabled:</ConfigLabel>
            <ToggleSwitch type='flat' checked={deciderEnabled} onChange={handleDeciderEnabled} />
          </ConfigRow>
          <ConfigRow>
            <ConfigLabel>Module:</ConfigLabel>
            <Select onChange={handleDeciderChange} value={decider}>
              {deciderList.map((decider, index) => (
                <option key={index} value={decider}>
                  {decider}
                </option>
              ))}
            </Select>
          </ConfigRow>
        </DeciderPanel>
        <PresenterPanel>
          <SmallerTitle>Presenter</SmallerTitle>
          <ConfigRow>
            <ConfigLabel>Enabled:</ConfigLabel>
            <ToggleSwitch type='flat' checked={presenterEnabled} onChange={setPresenterEnabled} />
          </ConfigRow>
          <ConfigRow>
            <ConfigLabel>Module:</ConfigLabel>
          </ConfigRow>
        </PresenterPanel>
        <TmsPanel>
          <SmallerTitle>TMS device</SmallerTitle>
          <ConfigRow>
            <ConfigLabel>Type:</ConfigLabel>
            <ConfigLabel>Multi-locus</ConfigLabel>
          </ConfigRow>
        </TmsPanel>
      </PipelinePanel>
      <SessionDisplay />
      <EegDisplay />
      <LatencyDisplay />
    </>
  )
}
