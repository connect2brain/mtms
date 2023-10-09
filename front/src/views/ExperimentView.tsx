import React, { useEffect, useState } from 'react'
import styled from 'styled-components'

import { TabBar } from 'styles/General'

import { LocationSelector, Point } from 'components/Experiment/LocationSelector'
import { AngleSelector } from 'components/Experiment/AngleSelector'
import { IntensitySelector } from 'components/Experiment/IntensitySelector'
import { TriggerSelector } from 'components/Experiment/TriggerSelector'

import { ToggleSwitch } from 'components/Experiment/ToggleSwitch'

import { SmallerTitle, ExperimentInput } from 'styles/ExperimentStyles'
import { StyledButton } from 'styles/General'

import { getMaximumIntensity, countValidTrials } from 'services/ros'

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
  gap: 1rem;
  padding: 1rem;
`

const styledPanel = `
  padding: 25px 0px 40px 35px;
  border-radius: 5px;
  background-color: #f7f7f7;
  box-shadow: 0px 3px 6px rgba(0, 0, 0, 0.1);/
`

const StimulationParametersPanel = styled.div`
  display: grid;
  grid-template-rows: repeat(1, 1fr);
  grid-template-columns: repeat(3, 1fr);
  gap: 15px;
  margin-bottom: 20px;
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
  gap: 1.5rem;
  width: 600px;
  height: 500px;
  ${styledPanel}
`

const IntensityPanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 3 / 4;
  gap: 1.5rem;
  width: 100px;
  height: 500px;
  ${styledPanel}
`

/* Config panels */
const ConfigPanel = styled.div`
  display: grid;
  grid-template-rows: repeat(1, 1fr);
  grid-template-columns: repeat(4, 1fr);
  width: 500px;
  height: 550px;
  gap: 20px;
`

const TriggerPanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 1 / 2;
  width: 272px;
  height: 240px;
  ${styledPanel}
`

const MepPanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 2 / 3;
  width: 272px;
  height: 240px;
  ${styledPanel}
`

const TrialsPanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 3 / 4;
  width: 300px;
  height: 240px;
  ${styledPanel}
`

const ExperimentPanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 4 / 5;
  width: 240px;
  height: 240px;
  ${styledPanel}
`

/* General config-related */
const ConfigRow = styled.div`
  display: flex;
  justify-content: flex-start;
  align-items: center;
  gap: 10px;
  margin-bottom: 10px;
  padding-right: 0px;
`

const ConfigLabel = styled.label`
  width: 300px;
  font-size: 14px;
`

/* Trials-related */
const IndentedLabel = styled(ConfigLabel)`
  padding-left: 15px;
`

const CloseConfigRow = styled(ConfigRow)`
  margin-bottom: 8px;
`

const GrayedOutPanel = styled.div<{ isGrayedOut: boolean }>`
  filter: ${props => props.isGrayedOut ? 'grayscale(100%)' : 'none'};
  opacity: ${props => props.isGrayedOut ? '0.5' : '1'};
  transition: filter 0.3s ease, opacity 0.3s ease;
`

/* Triggers */
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

/* TODO: Move type definitions elsewhere. */

type TriggerConfig = {
  enabled: boolean
  delay: number
}

type IntertrialInterval = {
  min: number
  max: number
  tolerance: number
}

type Metadata = {
  experiment_name: string
  subject_name: string
}

type Experiment = {
  metadata: Metadata

  trials: Trial[]
  intertrial_interval: IntertrialInterval

  randomize_trials: boolean
  wait_for_trigger: boolean
}

type Stimulus = {
  target: {
    displacement_x: number
    displacement_y: number
    rotation_angle: number
  }
  intensity: number
  triggers: TriggerConfig[]
}

type TimeWindow = {
  start: number
  end: number
}

type PreactivationCheck = {
  enabled: boolean
  time_window: TimeWindow
  voltage_range_limit: number
}

type MepConfiguration = {
  emg_channel: number
  time_window: TimeWindow
  preactivation_check: PreactivationCheck
}

type TrialConfig =  {
  analyze_mep: boolean
  mep_config: MepConfiguration
}

type Trial = {
  stimuli: Stimulus[]
  stimulus_times_since_trial_start: number[]
  config: TrialConfig
}

export const ExperimentView = () => {
  const [experimentName, setExperimentName] = useState<string>('')
  const [subjectName, setSubjectName] = useState<string>('')

  const [activeTab, setActiveTab] = useState<'singleLocation' | 'multipleLocations'>('singleLocation')

  const [selectedAngles, setSelectedAngles] = useState<number[]>([])
  const [selectedPoints, setSelectedPoints] = useState<Point[]>([])
  const [intensity, setIntensity] = useState(100)

  const [maximumIntensity, setMaximumIntensity] = useState(100)

  const [trigger1Enabled, setTrigger1Enabled] = useState<boolean>(false)
  const [trigger1Delay, setTrigger1Delay] = useState<number>(0)

  const [trigger2Enabled, setTrigger2Enabled] = useState<boolean>(false)
  const [trigger2Delay, setTrigger2Delay] = useState<number>(0)

  const [mepEnabled, setMepEnabled] = useState<boolean>(false)
  const [emgChannel, setEmgChannel] = useState<number>(1)

  const [numOfRepetitions, setNumOfRepetitions] = useState<number>(10)
  const [waitForTrigger, setWaitForTrigger] = useState<boolean>(false)

  const [itiMin, setItiMin] = useState<number>(3.5)
  const [itiMax, setItiMax] = useState<number>(4.5)

  const [numOfValidTrials, setNumOfValidTrials] = useState<number | null>(null)
  const [numOfTrials, setNumOfTrials] = useState<number>(0)

  const [debounceTimer, setDebounceTimer] = useState<NodeJS.Timeout | null>(null)

  const [isStartButtonDisabled, setIsStartButtonDisabled] = useState(false)

  const stimulate = () => {
    console.log('stimulate')
  }

  const changeTrigger1Enabled = (value: boolean) => {
    setTrigger1Enabled(value)
  }

  const changeTrigger1Delay = (value: number) => {
    setTrigger1Delay(value)
  }

  const changeTrigger2Enabled = (value: boolean) => {
    setTrigger2Enabled(value)
  }

  const changeTrigger2Delay = (value: number) => {
    setTrigger2Delay(value)
  }

  const changeMepEnabled = (value: boolean) => {
    setMepEnabled(value)
  }

  const changeEmgChannel = (event: React.ChangeEvent<HTMLInputElement>) => {
    const value = Math.max(Math.min(Number(event.target.value), 16), 1)

    /* TODO: Validate EMG channel value here, based on the number of available EMG channels,
         published in EegInfo topic by EEG bridge. */
    if (!isNaN(value)) {
      setEmgChannel(value)
    }
  }

  const changeNumOfRepetitions = (event: React.ChangeEvent<HTMLInputElement>) => {
    const value = Math.max(Number(event.target.value), 1)
    if (!isNaN(value)) {
      setNumOfRepetitions(value)
    }
  }

  const changeWaitForTrigger = (value: boolean) => {
    setWaitForTrigger(value)
  }

  const changeItiMin = (event: React.ChangeEvent<HTMLInputElement>) => {
    const value = Number(event.target.value)
    if (!isNaN(value) && value >= -99 && value <= 99) {
      setItiMin(value)
    }
  }

  const changeItiMax = (event: React.ChangeEvent<HTMLInputElement>) => {
    const value = Number(event.target.value)
    if (!isNaN(value) && value >= -99 && value <= 99) {
      setItiMax(value)
    }
  }

  const handleIntensityChange = (value: number) => {
    setIntensity(value)
    setIsStartButtonDisabled(true)
  }

  const formTrials = (): Trial[] => {
    const trials: Trial[] = []

    selectedPoints.forEach((point: Point) => {
      selectedAngles.forEach((angle: number) => {
        const triggers: TriggerConfig[] = [
          {
            enabled: trigger1Enabled,
            /* Delay is presented as milliseconds in the UI, transform to seconds. */
            delay: trigger1Delay / 1000
          },
          {
            enabled: trigger2Enabled,
            /* Delay is presented as milliseconds in the UI, transform to seconds. */
            delay: trigger2Delay / 1000
          }
        ]

        const stimulus: Stimulus = {
          target: {
            displacement_x: point.x,
            displacement_y: point.y,
            rotation_angle: angle
          },
          intensity: intensity,
          triggers: triggers
        }

        /* TODO: Hard-coded for now - make configurable. */
        const mep_config_time_window: TimeWindow = {
          start: 0.01,
          end: 0.04
        }

        /* TODO: Hard-coded for now - make configurable. */
        const preactivation_check_time_window: TimeWindow = {
          start: -0.05,
          end: -0.01
        }

        /* TODO: Hard-coded for now - make configurable. */
        const preactivation_check: PreactivationCheck = {
          enabled: false,
          time_window: preactivation_check_time_window,
          voltage_range_limit: 90
        }

        const mep_config: MepConfiguration = {
          /* 0-based indexing is internally used for EMG channels, hence decrement to allow
            the user to use 1-based indexing. */
          emg_channel: emgChannel - 1,
          time_window: mep_config_time_window,
          preactivation_check: preactivation_check
        }

        const trial_config: TrialConfig = {
          analyze_mep: mepEnabled,
          mep_config: mep_config
        }

        const trial: Trial = {
          stimuli: [stimulus],
          stimulus_times_since_trial_start: [0],
          config: trial_config
        }
        trials.push(trial)
      })
    })
    return trials
  }

  const formExperiment = (): Experiment => {
    const trials = formTrials()
    const repeatedTrials = ([] as Trial[]).concat(...Array(numOfRepetitions).fill(trials))

    const experiment: Experiment = {
      metadata: {
        experiment_name: experimentName,
        subject_name: subjectName
      },

      trials: repeatedTrials,
      intertrial_interval: {
        min: itiMin,
        max: itiMax,
        tolerance: 0.0
      },

      randomize_trials: true,
      wait_for_trigger: waitForTrigger
    }
    return experiment
  }

  const updateValidTrials = (experiment: Experiment) => {
    countValidTrials(experiment.trials, (numOfValidTrials) => {
      setNumOfValidTrials(numOfValidTrials)

      /* TODO: If there are several simultaneous callbacks, only the last of them should enable the
        start button; currently all of them enable it. */
      setIsStartButtonDisabled(false)
    })
  }

  useEffect(() => {
    if (activeTab === 'singleLocation') {
      setSelectedAngles([])
      setSelectedPoints([])

      setNumOfTrials(0)
      setNumOfValidTrials(null)

      setNumOfRepetitions(10)
    }
    if (activeTab === 'multipleLocations') {
      setNumOfRepetitions(1)
    }
  }, [activeTab])

  /* Updates the maximum intensity display. */
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

  /* Update # of valid trials. */
  useEffect(() => {
    if (selectedPoints.length == 0 || selectedAngles.length == 0) {
      return
    }
    const experiment: Experiment = formExperiment()

    const numOfTrials = experiment.trials.length
    setNumOfTrials(numOfTrials)

    setIsStartButtonDisabled(true)

    if (debounceTimer) {
      clearTimeout(debounceTimer)
    }

    const timer = setTimeout(() => {
      updateValidTrials(experiment)
    }, 200)

    setDebounceTimer(timer)

    // Cleanup function to clear the timer if the component is unmounted or if the effect runs again
    return () => {
      clearTimeout(timer)
    }
  }, [selectedAngles, selectedPoints, intensity, numOfRepetitions])

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

      <StimulationParametersPanel>
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
          <AngleSelector
            selectedAngles={selectedAngles}
            setSelectedAngles={setSelectedAngles}
          />
        }
        {activeTab === 'multipleLocations' &&
          <AngleSelector
            selectedAngles={selectedAngles}
            setSelectedAngles={setSelectedAngles}
            multiSelectMode={true}
          />
        }
        </AnglePanel>
        <IntensityPanel>
        {activeTab === 'singleLocation' &&
          <IntensitySelector
            min={0}
            max={150}
            showMaximumIntensity={true}
            maximumIntensity={maximumIntensity}
            onValueChange={handleIntensityChange}
          />
        }
        {activeTab === 'multipleLocations' &&
          <IntensitySelector
            min={0}
            max={150}
            showMaximumIntensity={false}
            maximumIntensity={0}
            onValueChange={handleIntensityChange}
          />
        }
        </IntensityPanel>
      </StimulationParametersPanel>
      <ConfigPanel>
          <TriggerPanel>
            <SmallerTitle>Triggers</SmallerTitle>
            <TriggerRow>
              <TriggerLabel>1</TriggerLabel>
              <TriggerSelector
                enabled={trigger1Enabled}
                enabledHandler={changeTrigger1Enabled}
                delay={trigger1Delay}
                delayHandler={changeTrigger1Delay}
              />
            </TriggerRow>
            <TriggerRow>
              <TriggerLabel>2</TriggerLabel>
              <TriggerSelector
                enabled={trigger2Enabled}
                enabledHandler={changeTrigger2Enabled}
                delay={trigger2Delay}
                delayHandler={changeTrigger2Delay}
              />
            </TriggerRow>
          </TriggerPanel>
          <MepPanel>
            <SmallerTitle>MEP analysis</SmallerTitle>
            <ConfigRow>
              <ConfigLabel>Enabled:</ConfigLabel>
              <ToggleSwitch
                type="flat"
                checked={mepEnabled}
                onChange={changeMepEnabled}
              />
            </ConfigRow>
            <GrayedOutPanel isGrayedOut={!mepEnabled}>
              <ConfigRow>
                <IndentedLabel>EMG channel:</IndentedLabel>
                <ExperimentInput
                  type="text"
                  value={emgChannel}
                  min={0}
                  max={16}
                  onChange={changeEmgChannel}
                  disabled={!mepEnabled}
                  />
              </ConfigRow>
            </GrayedOutPanel>
          </MepPanel>
          <TrialsPanel>
            <SmallerTitle>{activeTab === 'singleLocation' ? 'Trials' : 'Repetitions'}</SmallerTitle>
            <ConfigRow>
              <ConfigLabel># of {activeTab === 'singleLocation' ? 'trials' : 'repetitions'}:</ConfigLabel>
              <ExperimentInput
                type="text"
                value={numOfRepetitions}
                min={0}
                max={999}
                onChange={changeNumOfRepetitions}
              />
            </ConfigRow>
            <ConfigRow>
              <ConfigLabel>Wait for trigger:</ConfigLabel>
              <ToggleSwitch
                type="flat"
                checked={waitForTrigger}
                onChange={changeWaitForTrigger}
              />
            </ConfigRow>
            <GrayedOutPanel isGrayedOut={waitForTrigger || numOfRepetitions === 1}>
              <ConfigRow>
                <ConfigLabel>Interval (s)</ConfigLabel>
              </ConfigRow>
              <CloseConfigRow>
                <IndentedLabel>Min:</IndentedLabel>
                <ExperimentInput
                  type="number"
                  value={itiMin}
                  min={0}
                  step={0.1}
                  onChange={changeItiMin}
                  disabled={waitForTrigger || numOfRepetitions === 1}
                />
              </CloseConfigRow>
              <CloseConfigRow>
                <IndentedLabel>Max:</IndentedLabel>
                  <ExperimentInput
                    type="number"
                    value={itiMax}
                    min={0}
                    step={0.1}
                    onChange={changeItiMax}
                    disabled={waitForTrigger || numOfRepetitions === 1}
                />
              </CloseConfigRow>
            </GrayedOutPanel>
          </TrialsPanel>
          <ExperimentPanel>
            <SmallerTitle>Experiment</SmallerTitle>
            <ConfigRow>
              <ConfigLabel>Trials</ConfigLabel>
            </ConfigRow>
            <CloseConfigRow>
              <IndentedLabel>Total:</IndentedLabel>
              <ConfigLabel>{numOfTrials}</ConfigLabel>
            </CloseConfigRow>
            <CloseConfigRow>
              <IndentedLabel>Valid:</IndentedLabel>
              <ConfigLabel>{numOfValidTrials !== null ? numOfValidTrials : '\u2013'}</ConfigLabel>
            </CloseConfigRow>
            <CloseConfigRow></CloseConfigRow>
            <StyledButton
              onClick={stimulate}
              disabled={isStartButtonDisabled}
            >
              {isStartButtonDisabled ? 'Updating...' : 'Start'}
            </StyledButton>
          </ExperimentPanel>
        </ConfigPanel>
    </>
  )
}
