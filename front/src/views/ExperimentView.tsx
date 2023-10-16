import React, { useEffect, useState } from 'react'
import styled from 'styled-components'

import { TabBar } from 'styles/General'

import { LocationSelector, Point } from 'components/Experiment/LocationSelector'
import { AngleSelector } from 'components/Experiment/AngleSelector'
import { IntensitySelector } from 'components/Experiment/IntensitySelector'
import { ToggleSwitch } from 'components/Experiment/ToggleSwitch'

import { ValidatedInput } from 'components/ValidatedInput'

import { SmallerTitle } from 'styles/ExperimentStyles'
import { StyledButton } from 'styles/General'

import { getMaximumIntensity, countValidTrials, listProjects, performExperiment, pauseExperiment, resumeExperiment, setActiveProject } from 'services/ros'

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

const Input = styled.input`
  width: 300px;
  padding: 8px;
  border: 1px solid #ccc;
  border-radius: 4px;
  outline: none;
  transition: background-color 0.2s;

  &:focus {
    background-color: transparent;
  }
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
  grid-template-columns: repeat(5, 1fr);
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
  width: 270px;
  height: 240px;
  ${styledPanel}
`

const PausePanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 4 / 5;
  width: 270px;
  height: 240px;
  ${styledPanel}
`

const ExperimentPanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 5 / 6;
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

const DelayLabel = styled.label`
    margin-left: -8px;
    margin-right: 15px;
    font-size: 11px;
    font-weight: bold;
    text-align: right;
    color: 'black';
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

  autopause: boolean
  autopause_interval: number
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

enum StartButtonState {
  ReadyForExperiment,
  UpdatingTrialInfo,
  PerformingExperiment
}

/* Session storage utilities. */

const getData = (): any => {
  const data = sessionStorage.getItem('experiment')
  return data ? JSON.parse(data) : {}
}

const storeKey = (key: string, value: any) => {
  const currentData = getData()
  currentData[key] = value
  sessionStorage.setItem('experiment', JSON.stringify(currentData))
}

const getKey = (key: string, defaultValue: any): any => {
  const data = getData()
  return data[key] || defaultValue
}

export const ExperimentView = () => {
  const [projects, setProjects] = useState<string[]>([])

  const [selectedProject, setSelectedProject] = useState<string>(() => getKey('activeProject', ''))
  const [experimentName, setExperimentName] = useState<string>(() => getKey('experimentName', ''))
  const [subjectName, setSubjectName] = useState<string>(() => getKey('subjectName', ''))

  const [activeTab, setActiveTab] = useState<'singleLocation' | 'multipleLocations'>(() => getKey('activeTab', 'singleLocation'))

  const [selectedAngles, setSelectedAngles] = useState<number[]>(() => getKey('selectedAngles', []))
  const [selectedPoints, setSelectedPoints] = useState<Point[]>(() => getKey('selectedPoints', []))

  /* TODO: Currently, the initial value set here needs to match the initial value in IntensitySelector
    component - remove the dependency. */
  const [intensity, setIntensity] = useState<number>(() => getKey('intensity', 40))
  const [maximumIntensity, setMaximumIntensity] = useState<number>(() => getKey('maximumIntensity', 100))

  const [trigger1Enabled, setTrigger1Enabled] = useState<boolean>(() => getKey('trigger1Enabled', false))
  const [trigger1Delay, setTrigger1Delay] = useState<number>(() => getKey('trigger1Delay', 0))

  const [trigger2Enabled, setTrigger2Enabled] = useState<boolean>(() => getKey('trigger2Enabled', false))
  const [trigger2Delay, setTrigger2Delay] = useState<number>(() => getKey('trigger2Delay', 0))

  const [mepEnabled, setMepEnabled] = useState<boolean>(() => getKey('mepEnabled', true))
  const [emgChannel, setEmgChannel] = useState<number>(() => getKey('emgChannel', 1))

  const [numOfRepetitions, setNumOfRepetitions] = useState<number>(() => getKey('numOfRepetitions', 10))
  const [waitForTrigger, setWaitForTrigger] = useState<boolean>(() => getKey('waitForTrigger', false))

  const [itiMin, setItiMin] = useState<number>(() => getKey('itiMin', 3.5))
  const [itiMax, setItiMax] = useState<number>(() => getKey('itiMax', 4.5))

  const [autopause, setAutopause] = useState<boolean>(() => getKey('autopause', true))
  const [autopauseIntervalMinutes, setAutopauseIntervalMinutes] = useState<number>(() => getKey('autopauseIntervalMinutes', 10))

  const [numOfValidTrials, setNumOfValidTrials] = useState<number | null>(null)
  const [numOfTrials, setNumOfTrials] = useState<number | null>(null)
  const [duration, setDuration] = useState<number | null>(null)

  const [debounceTimer, setDebounceTimer] = useState<NodeJS.Timeout | null>(null)

  const [startButtonState, setStartButtonState] = useState(StartButtonState.ReadyForExperiment)

  const perform = () => {
    setStartButtonState(StartButtonState.PerformingExperiment)

    const experiment: Experiment = formExperiment()
    performExperiment(experiment, (trial_results, success) => {
      console.log(trial_results)
      setStartButtonState(StartButtonState.ReadyForExperiment)
    })
  }

  const handleProjectChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    const activeProject = event.target.value
    setSelectedProject(activeProject)

    /* TODO: The naming is a bit confusing here: setActiveProject makes a ROS service call; naming should
      somehow reflect that to distinguish it from setSelectedProject. */
    setActiveProject(activeProject, () => {
      console.log('Active project set to ' + activeProject)
    })
  }

  const handleIntensityChange = (intensity: number) => {
    setIntensity(intensity)
    setStartButtonState(StartButtonState.UpdatingTrialInfo)
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
      wait_for_trigger: waitForTrigger,

      autopause: autopause,
      autopause_interval: autopauseIntervalMinutes * 60
    }
    return experiment
  }

  const updateValidTrials = (experiment: Experiment) => {
    countValidTrials(experiment.trials, (numOfValidTrials) => {
      setNumOfValidTrials(numOfValidTrials)

      /* TODO: If there are several simultaneous callbacks, only the last of them should enable the
        start button; currently all of them enable it. */
      setStartButtonState(StartButtonState.ReadyForExperiment)
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

  /* Set list of projects. */
  useEffect(() => {
    listProjects((projects) => {
      setProjects(projects)
    })
  }, [])

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
      setNumOfTrials(null)
      setNumOfValidTrials(null)
      setDuration(null)

      return
    }
    const experiment: Experiment = formExperiment()

    const numOfTrials = experiment.trials.length
    setNumOfTrials(numOfTrials)

    setStartButtonState(StartButtonState.UpdatingTrialInfo)

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

  /* Update the experiment duration. */
  useEffect(() => {
    if (numOfValidTrials === null) {
      return
    }
    const duration = numOfValidTrials * (itiMin + itiMax) / 2
    setDuration(duration)
  }, [itiMin, itiMax, numOfValidTrials])

  /* Update session storage. */
  useEffect(() => {
    storeKey('activeProject', selectedProject)
  }, [selectedProject])

  useEffect(() => {
    storeKey('experimentName', experimentName)
  }, [experimentName])

  useEffect(() => {
    storeKey('subjectName', subjectName)
  }, [subjectName])

  useEffect(() => {
    storeKey('activeTab', activeTab)
  }, [activeTab])

  useEffect(() => {
    storeKey('selectedAngles', selectedAngles)
  }, [selectedAngles])

  useEffect(() => {
    storeKey('selectedPoints', selectedPoints)
  }, [selectedPoints])

  useEffect(() => {
    storeKey('intensity', intensity)
  }, [intensity])

  useEffect(() => {
    storeKey('maximumIntensity', maximumIntensity)
  }, [maximumIntensity])

  useEffect(() => {
    storeKey('trigger1Enabled', trigger1Enabled)
  }, [trigger1Enabled])

  useEffect(() => {
    storeKey('trigger1Delay', trigger1Delay)
  }, [trigger1Delay])

  useEffect(() => {
    storeKey('trigger2Enabled', trigger2Enabled)
  }, [trigger2Enabled])

  useEffect(() => {
    storeKey('trigger2Delay', trigger2Delay)
  }, [trigger2Delay])

  useEffect(() => {
    storeKey('mepEnabled', mepEnabled)
  }, [mepEnabled])

  useEffect(() => {
    storeKey('emgChannel', emgChannel)
  }, [emgChannel])

  useEffect(() => {
    storeKey('numOfRepetitions', numOfRepetitions)
  }, [numOfRepetitions])

  useEffect(() => {
    storeKey('waitForTrigger', waitForTrigger)
  }, [waitForTrigger])

  useEffect(() => {
    storeKey('itiMin', itiMin)
  }, [itiMin])

  useEffect(() => {
    storeKey('itiMax', itiMax)
  }, [itiMax])

  useEffect(() => {
    storeKey('autopause', autopause)
  }, [autopause])

  useEffect(() => {
    storeKey('autopauseIntervalMinutes', autopauseIntervalMinutes)
  }, [autopauseIntervalMinutes])

  /* Utilities */
  const formatDuration = (duration: number): string => {
    duration = Math.round(duration)

    const hours = Math.floor(duration / 3600)
    const minutes = Math.floor((duration % 3600) / 60)
    const seconds = duration % 60

    let result = ''

    if (hours > 0) {
      result += `${hours} h `
    }
    if (minutes > 0) {
      result += `${minutes} min `
    }
    if (seconds > 0 || result === '') {
      result += `${seconds} s`
    }

    return result.trim()
  }

  const startButtonStateToString = (startButtonState: StartButtonState): string => {
    switch (startButtonState) {
      case StartButtonState.UpdatingTrialInfo:
        return 'Updating...'
      case StartButtonState.ReadyForExperiment:
        return 'Start'
      case StartButtonState.PerformingExperiment:
        return 'Pause'
    }
  }

  const runStartButtonAction = (startButtonState: StartButtonState) => {
    switch (startButtonState) {
      case StartButtonState.UpdatingTrialInfo:
        break
      case StartButtonState.ReadyForExperiment:
        perform()
        break
      case StartButtonState.PerformingExperiment:
        pauseExperiment(() => {
          console.log('paused')
        })
        break
    }
  }


  return (
    <>
      <ExperimentMetadata>
        <InputRow>
          <Label>Project:</Label>
          <Select onChange={handleProjectChange} value={selectedProject}>
          {projects.map((project, index) => (
            <option key={index} value={project}>
              {project}
            </option>
          ))}
          </Select>
        </InputRow>

        <InputRow>
          <Label>Experiment:</Label>
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
              <ToggleSwitch
                type="flat"
                checked={trigger1Enabled}
                onChange={setTrigger1Enabled}
              />
              <GrayedOutPanel isGrayedOut={!trigger1Enabled}>
                <DelayLabel>Delay (ms)</DelayLabel>
              </GrayedOutPanel>
              <GrayedOutPanel isGrayedOut={!trigger1Enabled}>
                <ValidatedInput
                  value={trigger1Delay}
                  min={-99}
                  max={99}
                  defaultValue={0}
                  onChange={setTrigger1Delay}
                  disabled={!trigger1Enabled}
                />
              </GrayedOutPanel>
            </TriggerRow>
            <TriggerRow>
            <TriggerLabel>2</TriggerLabel>
              <ToggleSwitch
                type="flat"
                checked={trigger2Enabled}
                onChange={setTrigger2Enabled}
              />
              <GrayedOutPanel isGrayedOut={!trigger2Enabled}>
                <DelayLabel>Delay (ms)</DelayLabel>
              </GrayedOutPanel>
              <GrayedOutPanel isGrayedOut={!trigger2Enabled}>
                <ValidatedInput
                  value={trigger2Delay}
                  min={-99}
                  max={99}
                  defaultValue={0}
                  onChange={setTrigger2Delay}
                  disabled={!trigger2Enabled}
                />
              </GrayedOutPanel>
            </TriggerRow>
          </TriggerPanel>
          <MepPanel>
            <SmallerTitle>MEP analysis</SmallerTitle>
            <ConfigRow>
              <ConfigLabel>Enabled:</ConfigLabel>
              <ToggleSwitch
                type="flat"
                checked={mepEnabled}
                onChange={setMepEnabled}
              />
            </ConfigRow>
            <GrayedOutPanel isGrayedOut={!mepEnabled}>
              <ConfigRow>
                <IndentedLabel>EMG channel:</IndentedLabel>
                {/*
                  TODO: Validate EMG channel by passing a prop to ValidatedInput, based on the
                  number of available EMG channels, published in EegInfo topic by EEG bridge.
                */}
                <ValidatedInput
                  type="text"
                  value={emgChannel}
                  defaultValue={1}
                  min={1}
                  max={16}
                  onChange={setEmgChannel}
                  disabled={!mepEnabled}
                  />
              </ConfigRow>
            </GrayedOutPanel>
          </MepPanel>
          <TrialsPanel>
            <SmallerTitle>{activeTab === 'singleLocation' ? 'Trials' : 'Repetitions'}</SmallerTitle>
            <ConfigRow>
              <ConfigLabel># of {activeTab === 'singleLocation' ? 'trials' : 'repetitions'}:</ConfigLabel>
              <ValidatedInput
                type="text"
                value={numOfRepetitions}
                defaultValue={1}
                min={1}
                max={999}
                onChange={setNumOfRepetitions}
              />
            </ConfigRow>
            <ConfigRow>
              <ConfigLabel>Wait for trigger:</ConfigLabel>
              <ToggleSwitch
                type="flat"
                checked={waitForTrigger}
                onChange={setWaitForTrigger}
              />
            </ConfigRow>
            <GrayedOutPanel isGrayedOut={waitForTrigger || numOfTrials === null || numOfTrials < 2}>
              <ConfigRow>
                <ConfigLabel>Interval (s)</ConfigLabel>
              </ConfigRow>
              <CloseConfigRow>
                <IndentedLabel>Min:</IndentedLabel>
                <ValidatedInput
                  type="number"
                  value={itiMin}
                  defaultValue={3.0}
                  min={0.1}
                  max={100}
                  step={0.1}
                  onChange={setItiMin}
                  disabled={waitForTrigger || numOfTrials === null || numOfTrials < 2}
                />
              </CloseConfigRow>
              <CloseConfigRow>
                <IndentedLabel>Max:</IndentedLabel>
                  <ValidatedInput
                    type="number"
                    value={itiMax}
                    defaultValue={4.0}
                    min={0.1}
                    max={100}
                    step={0.1}
                    onChange={setItiMax}
                    disabled={waitForTrigger || numOfTrials === null || numOfTrials < 2}
                />
              </CloseConfigRow>
            </GrayedOutPanel>
          </TrialsPanel>
          <PausePanel>
            <SmallerTitle>Pause</SmallerTitle>
            <ConfigRow>
              <ConfigLabel>Automatic:</ConfigLabel>
              <ToggleSwitch
                type="flat"
                checked={autopause}
                onChange={setAutopause}
              />
            </ConfigRow>
            <GrayedOutPanel isGrayedOut={!autopause}>
              <ConfigRow>
                <IndentedLabel>Interval (min):</IndentedLabel>
                <ValidatedInput
                  type="number"
                  value={autopauseIntervalMinutes}
                  defaultValue={10}
                  min={1}
                  max={99}
                  onChange={setAutopauseIntervalMinutes}
                  disabled={!autopause}
                />
              </ConfigRow>
            </GrayedOutPanel>
          </PausePanel>
          <ExperimentPanel>
            <SmallerTitle>Experiment</SmallerTitle>
            <ConfigRow>
              <ConfigLabel>Trials</ConfigLabel>
            </ConfigRow>
            <CloseConfigRow>
              <IndentedLabel>Total:</IndentedLabel>
              <ConfigLabel>{numOfTrials !== null ? numOfTrials : '\u2013'}</ConfigLabel>
            </CloseConfigRow>
            <CloseConfigRow>
              <IndentedLabel>Valid:</IndentedLabel>
              <ConfigLabel>{numOfValidTrials !== null ? numOfValidTrials : '\u2013'}</ConfigLabel>
            </CloseConfigRow>
            <ConfigRow>
              <ConfigLabel>Experiment</ConfigLabel>
            </ConfigRow>
            <CloseConfigRow>
              <IndentedLabel>Duration:</IndentedLabel>
              <ConfigLabel>{duration ? formatDuration(duration) : '\u2013'}</ConfigLabel>
            </CloseConfigRow>
            <CloseConfigRow></CloseConfigRow>
            <StyledButton
              onClick={() => runStartButtonAction(startButtonState)}
              disabled={startButtonState === StartButtonState.UpdatingTrialInfo}
            >
              {startButtonStateToString(startButtonState)}
            </StyledButton>
          </ExperimentPanel>
        </ConfigPanel>
    </>
  )
}
