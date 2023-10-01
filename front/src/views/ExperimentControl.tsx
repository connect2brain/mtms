import React, { useEffect, useState } from 'react'
import styled from 'styled-components'

import { LocationSelector, Point } from 'components/Experiment/LocationSelector'
import { AngleSelector } from 'components/Experiment/AngleSelector'
import { IntensitySelector } from 'components/Experiment/IntensitySelector'
import { TriggerSelector } from 'components/Experiment/TriggerSelector'

import { ToggleSwitch } from 'components/Experiment/ToggleSwitch'

import { SmallerTitle } from 'components/Experiment/Styles'

import { getMaximumIntensity } from 'services/ros'

/* Styles for inputs for experiment metadata (= experiment and subject name) */
const ExperimentMetadata = styled.div`
  margin-bottom: 40px;
`

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

const Input = styled.input`
  width: 300px;
  padding: 8px;
  border: 1px solid #ccc;
  border-radius: 4px;
  outline: none;
  transition: background-color 0.2s;

  &:focus {
    background-color: transparent;  // resets the background to transparent when the input is focused
  }
`

const Wrapper = styled.div`
  display: grid;
  grid-template-columns: 1fr 1fr;
  grid-template-rows: auto 1fr;
  gap: 1rem;
  padding: 1rem;
`

const styledPanel = `
  padding: 25px 0px 40px 35px;
  border-radius: 5px;
  background-color: #f7f7f7;
  box-shadow: 0px 3px 6px rgba(0, 0, 0, 0.1);/
`

const TabBar = styled.div`
  margin: 0.5rem;

  a {
    text-decoration: none;
    color: #505050;
    padding: 0.5rem;
    display: inline-block;
    transition: color 0.3s ease;

    &:hover {
      color: #303030;
    }

    &.active {
      color: #222222;
      font-weight: bold;
    }
  }
`

const GridPanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 1 / 2;
  width: 600px;
  height: 500px;
  ${styledPanel}
`

const AnglePanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 2 / 3;
  display: flex;
  flex-direction: column;
  gap: 1.5rem;
  width: 600px;
  height: 500px;
  ${styledPanel}
`

const IntensityPanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 3 / 3;
  display: flex;
  flex-direction: column;
  gap: 1.5rem;
  width: 100px;
  height: 500px;
  ${styledPanel}
`

const ConfigPanel = styled.div`
  grid-row: 2 / 2;
  grid-column: 1 / 3;
  display: flex;
  flex-direction: row;
  width: 600px;
  height: 500px;
  ${styledPanel}
`

const TriggerPanel = styled.div`
  grid-row: 1 / 1;
  grid-column: 1 / 2;
  width: 240px;
  height: 160px;
  ${styledPanel}
`

const MepPanel = styled.div`
  grid-row: 1 / 1;
  grid-column: 2 / 2;
  margin-left: 25px;
  width: 240px;
  height: 160px;
  ${styledPanel}
`

const TriggerRow = styled.div`
  display: flex;
  justify-content: flex-start;
  align-items: center;
  gap: 10px;
  margin-bottom: 10px;
`

const TriggerLabel = styled.label`
  width: 0px;
  font-size: 14px;
  font-weight: bold;
  text-align: left;
  display: inline-block;
`

const MepRow = styled.div`
  display: flex;
  justify-content: flex-start;
  align-items: center;
  gap: 10px;
  margin-bottom: 10px;
`

const MepEnabledLabel = styled.label`
  width: 200px;
  font-size: 15px;
  text-align: right;
  margin-right: 30px;
`

export const ExperimentControl = () => {
  const [activeTab, setActiveTab] = useState<'singleLocation' | 'multipleLocations'>('singleLocation')
  const [selectedAngles, setSelectedAngles] = useState<number[]>([])
  const [selectedPoints, setSelectedPoints] = useState<Point[]>([])
  const [maximumIntensity, setMaximumIntensity] = useState(100)

  const [experimentName, setExperimentName] = useState<string>('')
  const [subjectName, setSubjectName] = useState<string>('')

  const [trigger1Enabled, setTrigger1Enabled] = useState<boolean>(false)
  const [trigger1Delay, setTrigger1Delay] = useState<number>(0)

  const [trigger2Enabled, setTrigger2Enabled] = useState<boolean>(false)
  const [trigger2Delay, setTrigger2Delay] = useState<number>(0)

  const [mepEnabled, setMepEnabled] = useState<boolean>(false)

  const handleTrigger1EnabledChange = (value: boolean) => {
    setTrigger1Enabled(value)
  }

  const handleTrigger1DelayChange = (value: number) => {
    setTrigger1Delay(value)
  }

  const handleTrigger2EnabledChange = (value: boolean) => {
    setTrigger2Enabled(value)
  }

  const handleTrigger2DelayChange = (value: number) => {
    setTrigger2Delay(value)
  }

  const handleMepEnabled = (value: boolean) => {
    setMepEnabled(value)
  }

  const handleIntensityChange = (value: number) => {
    console.log(`Selected intensity: ${value} V/m`)
  }

  useEffect(() => {
    if (activeTab === 'singleLocation') {
      setSelectedAngles([])
      setSelectedPoints([])
    }
  }, [activeTab])

  useEffect(() => {
    if (selectedPoints.length === 1 && selectedAngles.length === 1) {
      const x: number = selectedPoints[0].x
      const y: number = selectedPoints[0].y
      const angle: number = selectedAngles[0]
      getMaximumIntensity(x, y, angle, (maximum_intensity) => {
        setMaximumIntensity(maximum_intensity)
      })
    }
  }, [selectedAngles, selectedPoints])

  return (
    <>
      <ExperimentMetadata>
        <InputRow>
          <Label>Name:</Label>
          <Input
            type="text"
            placeholder="E.g., Resting motor threshold experiment 1"
            value={experimentName}
            onChange={e => setExperimentName(e.target.value)}
            style={{ backgroundColor: experimentName ? 'transparent' : 'lightgray' }}
          />
        </InputRow>
        <InputRow>
          <Label>Subject:</Label>
          <Input
            type="text"
            placeholder="E.g., Subject 1"
            value={subjectName}
            onChange={e => setSubjectName(e.target.value)}
            style={{ backgroundColor: subjectName ? 'transparent' : 'lightgray' }}
          />
        </InputRow>
      </ExperimentMetadata>

      <TabBar>
        <a
          href="#"
          onClick={() => setActiveTab('singleLocation')}
          className={activeTab === 'singleLocation' ? 'active' : ''}
        >
          Single Location
        </a>
        <a
          href="#"
          onClick={() => setActiveTab('multipleLocations')}
          className={activeTab === 'multipleLocations' ? 'active' : ''}
        >
          Multiple Locations
        </a>
      </TabBar>

      <Wrapper>
        <GridPanel>
          {activeTab === 'singleLocation' &&
          <LocationSelector
            selectedPoints={selectedPoints}
            setSelectedPoints={setSelectedPoints}
          />}
          {activeTab === 'multipleLocations' && <LocationSelector
            selectedPoints={selectedPoints}
            setSelectedPoints={setSelectedPoints}
            multiSelectMode={true}
          />}
        </GridPanel>
        <AnglePanel>
        {activeTab === 'singleLocation' &&
          <AngleSelector selectedAngles={selectedAngles} setSelectedAngles={setSelectedAngles} />
        }
        {activeTab === 'multipleLocations' &&
          <AngleSelector selectedAngles={selectedAngles} setSelectedAngles={setSelectedAngles} multiSelectMode={true} />
        }
        </AnglePanel>
        <IntensityPanel>
        {activeTab === 'singleLocation' &&
          <IntensitySelector
            min={0}
            max={150}
            maximumIntensity={maximumIntensity}
            onValueChange={handleIntensityChange}
          />
        }
        {activeTab === 'multipleLocations' &&
          <IntensitySelector
            min={0}
            max={150}
            maximumIntensity={maximumIntensity}
            onValueChange={handleIntensityChange}
          />
        }
        </IntensityPanel>
        <ConfigPanel>
          <TriggerPanel>
            <SmallerTitle>Triggers</SmallerTitle>
            <TriggerRow>
              <TriggerLabel>1</TriggerLabel>
              <TriggerSelector
                enabled={trigger1Enabled}
                enabledHandler={handleTrigger1EnabledChange}
                delay={trigger1Delay}
                delayHandler={handleTrigger1DelayChange}
              />
            </TriggerRow>
            <TriggerRow>
              <TriggerLabel>2</TriggerLabel>
              <TriggerSelector
                enabled={trigger2Enabled}
                enabledHandler={handleTrigger2EnabledChange}
                delay={trigger2Delay}
                delayHandler={handleTrigger2DelayChange}
              />
            </TriggerRow>
          </TriggerPanel>
          <MepPanel>
            <SmallerTitle>MEP analysis</SmallerTitle>
            <MepRow>
              <MepEnabledLabel>Enabled:</MepEnabledLabel>
              <ToggleSwitch
                type="flat"
                checked={mepEnabled}
                onChange={handleMepEnabled}
              />
            </MepRow>
          </MepPanel>
        </ConfigPanel>
      </Wrapper>
    </>
  )
}
