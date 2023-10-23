import React, { useContext, useEffect, useState, useRef } from 'react'
import styled from 'styled-components'

import { TabBar } from 'styles/General'

import { SmallerTitle } from 'styles/ExperimentStyles'
import { StyledPanel, StyledButton, StyledRedButton } from 'styles/General'

import { listProjects, setActiveProject, setPreprocessorRos } from 'services/ros'

import { PipelineContext } from 'providers/PipelineProvider'
import { ProjectContext } from 'providers/ProjectProvider'

const InputRow = styled.div`
  display: flex;
  justify-content: flex-start;
  align-items: center;
  gap: 10px;
  margin-bottom: 10px;
`

const Label = styled.label`
  width: 150px;
  text-align: left;
  margin-right: 10px;
  margin-left: 30px;
  display: inline-block;
`

const Select = styled.select`
  width: 318px;
  padding: 8px;
  border: 1px solid #ccc;
  border-radius: 4px;
  outline: none;
  transition: background-color 0.2s;
  appearance: none;

  &:focus {
    background-color: transparent;
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
  const { preprocessors } = useContext(PipelineContext)
  const { activeProject } = useContext(ProjectContext)

  const [projects, setProjects] = useState<string[]>([])
  const [preprocessor, setPreprocessor] = useState<string>('')

  const handleProjectChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    const newActiveProject = event.target.value
    setActiveProject(newActiveProject, () => {
      console.log('Active project set to ' + newActiveProject)
    })
  }

  const handlePreprocessorChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    const preprocessor = event.target.value
    setPreprocessor(preprocessor)

    setPreprocessorRos(preprocessor, () => {
      console.log('Preprocessor set to ' + preprocessor)
    })
  }

  /* Set list of projects. */
  useEffect(() => {
    listProjects((projects) => {
      setProjects(projects)
    })
  }, [])

  return (
    <>
      <InputRow>
        <Label>Project:</Label>
        <Select onChange={handleProjectChange} value={activeProject}>
        {projects.map((project, index) => (
          <option key={index} value={project}>
            {project}
          </option>
        ))}
        </Select>
      </InputRow>

      <InputRow>
        <Label>Preprocessor:</Label>
        <Select onChange={handlePreprocessorChange} value={preprocessor}>
        {preprocessors.map((preprocessor, index) => (
          <option key={index} value={preprocessor}>
            {preprocessor}
          </option>
        ))}
        </Select>
      </InputRow>
     </>
  )
}
