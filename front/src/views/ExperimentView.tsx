import React, { useContext, useEffect, useState, useRef } from 'react'
import styled from 'styled-components'

import { TabBar, Select, GrayedOutPanel } from 'styles/General'

import { LocationSelector, Point } from 'components/Experiment/LocationSelector'
import { AngleSelector } from 'components/Experiment/AngleSelector'
import { IntensitySelector } from 'components/Experiment/IntensitySelector'
import { ToggleSwitch } from 'components/Experiment/ToggleSwitch'

import { ValidatedInput } from 'components/ValidatedInput'

import { SmallerTitle } from 'styles/ExperimentStyles'
import {
  StyledPanel,
  StyledButton,
  StyledRedButton,
  ProjectRow,
  ConfigRow,
  CloseConfigRow,
  ConfigLabel,
  IndentedLabel,
} from 'styles/General'

import {
  getMaximumIntensity,
  countValidTrials,
  listProjects,
  performExperiment,
  pauseExperiment,
  resumeExperiment,
  cancelExperiment,
  setActiveProject,
} from 'ros/ros'

import { SystemContext } from 'providers/SystemProvider'
import { ProjectContext } from 'providers/ProjectProvider'
import { HealthcheckContext, HealthcheckStatus } from 'providers/HealthcheckProvider'
import { ConfigContext } from 'providers/ConfigProvider'

import { formatTime } from 'utils/utils'

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
    background-color: transparent;
  }
`

const StimulationParametersPanel = styled.div`
  display: grid;
  grid-template-rows: repeat(1, 1fr);
  grid-template-columns: repeat(3, 1fr);
  gap: 15px;
  margin-bottom: 20px;
`

const GridPanel = styled(StyledPanel)`
  grid-row: 1 / 2;
  grid-column: 1 / 2;
  width: 600px;
  height: 500px;
`

const AnglePanel = styled(StyledPanel)`
  grid-row: 1 / 2;
  grid-column: 2 / 3;
  gap: 1.5rem;
  width: 600px;
  height: 500px;
`

const IntensityPanel = styled(StyledPanel)`
  grid-row: 1 / 2;
  grid-column: 3 / 4;
  gap: 1.5rem;
  width: 100px;
  height: 500px;
`

/* Config panels */
const ConfigPanel = styled.div`
  display: grid;
  grid-template-rows: repeat(1, 1fr);
  grid-template-columns: repeat(6, 1fr);
  width: 600px;
  height: 550px;
  gap: 20px;
`

const TriggerPanel = styled(StyledPanel)`
  grid-row: 1 / 2;
  grid-column: 1 / 2;
  width: 272px;
  height: 240px;
`

const MepPanel = styled(StyledPanel)`
  grid-row: 1 / 2;
  grid-column: 2 / 3;
  width: 272px;
  height: 240px;
`

const TrialsPanel = styled(StyledPanel)`
  grid-row: 1 / 2;
  grid-column: 3 / 4;
  width: 270px;
  height: 240px;
`

const PausePanel = styled(StyledPanel)`
  grid-row: 1 / 2;
  grid-column: 4 / 5;
  width: 270px;
  height: 240px;
`

const ExperimentPanel = styled(StyledPanel)`
  grid-row: 1 / 2;
  grid-column: 5 / 6;
  width: 240px;
  height: 240px;
`

const StatusPanel = styled(StyledPanel)`
  grid-row: 1 / 2;
  grid-column: 6 / 7;
  width: 240px;
  height: 300px;
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
    algorithm: {
      value: number
    }
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

type TrialConfig = {
  analyze_mep: boolean
  mep_config: MepConfiguration
}

type Trial = {
  stimuli: Stimulus[]
  stimulus_times_since_trial_start: number[]
  config: TrialConfig
}

enum StartButtonState {
  Start,
  Updating,
  Pausing,
  Pause,
  Resume,
}

enum CancelButtonState {
  Cancel,
  Canceling,
}

enum ExperimentState {
  NotRunning,
  Running,
  Paused,
  Canceled,
}

enum ExperimentTab {
  SingleLocation,
  MultipleLocations,
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
  return key in data ? data[key] : defaultValue
}

export const ExperimentView = () => {
  const { activeProject } = useContext(ProjectContext)

  const { mepHealthcheck } = useContext(HealthcheckContext)
  const [mepHealthcheckOk, setMepHealthcheckOk] = useState(false)

  const { systemState } = useContext(SystemContext)
  const { targetingAlgorithm } = useContext(ConfigContext)

  const [projects, setProjects] = useState<string[]>([])

  const [experimentName, setExperimentName] = useState<string>(() => getKey('experimentName', ''))
  const [subjectName, setSubjectName] = useState<string>(() => getKey('subjectName', ''))

  const [activeTab, setActiveTab] = useState<ExperimentTab>(() => getKey('activeTab', ExperimentTab.SingleLocation))

  const [selectedAngles, setSelectedAngles] = useState<number[]>(() => getKey('selectedAngles', [0]))
  const [selectedPoints, setSelectedPoints] = useState<Point[]>(() => getKey('selectedPoints', [{ x: 0, y: 0 }]))

  const [highlightedPoints, setHighlightedPoints] = useState<Point[]>([])
  const [highlightedAngles, setHighlightedAngles] = useState<number[]>([])

  /* TODO: Currently, the initial value set here needs to match the initial value in IntensitySelector
    component - remove the dependency. */
  const [intensity, setIntensity] = useState<number>(() => getKey('intensity', 10))
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
  const [autopauseIntervalMinutes, setAutopauseIntervalMinutes] = useState<number>(() =>
    getKey('autopauseIntervalMinutes', 10),
  )

  const [numOfValidTrials, setNumOfValidTrials] = useState<number | null>(null)
  const [numOfTrials, setNumOfTrials] = useState<number | null>(null)
  const [duration, setDuration] = useState<number | null>(null)

  const debounceTimerRef = useRef<NodeJS.Timeout | null>(null)

  const [startButtonState, setStartButtonState] = useState(StartButtonState.Start)
  const [cancelButtonState, setCancelButtonState] = useState(CancelButtonState.Cancel)

  const [trialNumber, setTrialNumber] = useState<number | null>(null)
  const [attemptNumber, setAttemptNumber] = useState<number | null>(null)
  const [experimentState, setExperimentState] = useState<number>(ExperimentState.NotRunning)

  const visualizeFeedback = (feedback: any) => {
    const target = feedback.trial.stimuli[0].target
    const x: number = target.displacement_x
    const y: number = target.displacement_y
    const angle: number = target.rotation_angle

    setHighlightedPoints([{ x: x, y: y }])
    setHighlightedAngles([angle])
  }

  const perform = () => {
    const experiment: Experiment = formExperiment()

    const done_callback = (trial_results: any, success: boolean) => {
      setTrialNumber(null)
      setAttemptNumber(null)
      setExperimentState(ExperimentState.NotRunning)
      setCancelButtonState(CancelButtonState.Cancel)
      setHighlightedPoints([])
      setHighlightedAngles([])
    }

    const feedback_callback = (feedback: any) => {
      visualizeFeedback(feedback)
      setTrialNumber(feedback.trial_number)
      setAttemptNumber(feedback.attempt_number)
      setExperimentState(feedback.experiment_state.value)
    }
    performExperiment(experiment, done_callback, feedback_callback)
  }

  const handleProjectChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    const newActiveProject = event.target.value
    setActiveProject(newActiveProject, () => {
      console.log('Active project set to ' + newActiveProject)
    })
  }

  const handleIntensityChange = (intensity: number) => {
    setIntensity(intensity)
    setStartButtonState(StartButtonState.Updating)
  }

  const formTrials = (): Trial[] => {
    const trials: Trial[] = []

    selectedPoints.forEach((point: Point) => {
      selectedAngles.forEach((angle: number) => {
        const triggers: TriggerConfig[] = [
          {
            enabled: trigger1Enabled,
            /* Delay is presented as milliseconds in the UI, transform to seconds. */
            delay: trigger1Delay / 1000,
          },
          {
            enabled: trigger2Enabled,
            /* Delay is presented as milliseconds in the UI, transform to seconds. */
            delay: trigger2Delay / 1000,
          },
        ]

        const stimulus: Stimulus = {
          target: {
            displacement_x: point.x,
            displacement_y: point.y,
            rotation_angle: angle,
            algorithm: {
              value: targetingAlgorithm,
            },
          },
          intensity: intensity,
          triggers: triggers,
        }

        /* TODO: Hard-coded for now - make configurable. */
        const mep_config_time_window: TimeWindow = {
          start: 0.01,
          end: 0.04,
        }

        /* TODO: Hard-coded for now - make configurable. */
        const preactivation_check_time_window: TimeWindow = {
          start: -0.05,
          end: -0.01,
        }

        /* TODO: Hard-coded for now - make configurable. */
        const preactivation_check: PreactivationCheck = {
          enabled: false,
          time_window: preactivation_check_time_window,
          voltage_range_limit: 90,
        }

        const mep_config: MepConfiguration = {
          /* 0-based indexing is internally used for EMG channels, hence decrement to allow
            the user to use 1-based indexing. */
          emg_channel: emgChannel - 1,
          time_window: mep_config_time_window,
          preactivation_check: preactivation_check,
        }

        const trial_config: TrialConfig = {
          /* Override mepEnabled if MEP healthcheck is not ok. */
          analyze_mep: mepHealthcheckOk ? mepEnabled : false,
          mep_config: mep_config,
        }

        const trial: Trial = {
          stimuli: [stimulus],
          stimulus_times_since_trial_start: [0],
          config: trial_config,
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
        subject_name: subjectName,
      },

      trials: repeatedTrials,
      intertrial_interval: {
        min: itiMin,
        max: itiMax,
        tolerance: 0.0,
      },

      randomize_trials: true,
      wait_for_trigger: waitForTrigger,

      autopause: autopause,
      autopause_interval: autopauseIntervalMinutes * 60,
    }
    return experiment
  }

  const callCountRef = useRef(0)

  const updateValidTrials = (experiment: Experiment) => {
    callCountRef.current += 1
    const currentCallCount = callCountRef.current

    countValidTrials(experiment.trials, (numOfValidTrials) => {
      if (currentCallCount === callCountRef.current) {
        setNumOfValidTrials(numOfValidTrials)
        setStartButtonState(StartButtonState.Start)
      }
    })
  }

  const updateValidTrialsWithDebounce = (experiment: Experiment) => {
    setStartButtonState(StartButtonState.Updating)

    if (debounceTimerRef.current) {
      clearTimeout(debounceTimerRef.current)
    }

    const timer = setTimeout(() => {
      updateValidTrials(experiment)
    }, 500)

    debounceTimerRef.current = timer

    // Cleanup function to clear the timer if the component is unmounted or if the effect runs again
    return () => {
      clearTimeout(timer)
    }
  }

  const changeActiveTab = (activeTab: ExperimentTab) => {
    setActiveTab(activeTab)

    switch (activeTab) {
      case ExperimentTab.SingleLocation:
        setSelectedAngles([0])
        setSelectedPoints([{ x: 0, y: 0 }])

        setNumOfTrials(0)
        setNumOfValidTrials(null)

        setNumOfRepetitions(10)

        break
      case ExperimentTab.MultipleLocations:
        setNumOfRepetitions(1)

        break
    }
  }

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
      getMaximumIntensity(x, y, angle, targetingAlgorithm, (maximum_intensity) => {
        setMaximumIntensity(maximum_intensity)
      })
    }
  }, [selectedAngles, selectedPoints, targetingAlgorithm])

  /* Update the number of valid trials. */
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

    updateValidTrialsWithDebounce(experiment)
  }, [selectedAngles, selectedPoints, intensity, numOfRepetitions, targetingAlgorithm])

  /* Update the experiment duration. */
  useEffect(() => {
    if (numOfValidTrials === null) {
      return
    }
    const duration = (numOfValidTrials * (itiMin + itiMax)) / 2
    setDuration(duration)
  }, [itiMin, itiMax, numOfValidTrials])

  /* Update experiment state. */
  useEffect(() => {
    switch (experimentState) {
      case ExperimentState.NotRunning:
        setStartButtonState(StartButtonState.Start)
        break

      case ExperimentState.Running:
        setStartButtonState(StartButtonState.Pause)
        break

      case ExperimentState.Paused:
        setStartButtonState(StartButtonState.Resume)
        break
    }
  }, [experimentState])

  /* Update MEP healthcheck ok status. */
  useEffect(() => {
    setMepHealthcheckOk(mepHealthcheck?.status.value === HealthcheckStatus.READY)
  }, [mepHealthcheck])

  /* Update session storage. */
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
  const formatExperimentState = (): string => {
    switch (experimentState) {
      case ExperimentState.NotRunning:
        return 'Not running'
      case ExperimentState.Running:
        return 'Running'
      case ExperimentState.Paused:
        return 'Paused'
      case ExperimentState.Canceled:
        return 'Canceled'
    }
    return 'Unknown'
  }

  const formatTrialNumber = () => {
    if (trialNumber === null || numOfValidTrials === null) {
      return '\u2013'
    }
    return trialNumber.toString() + '/' + numOfValidTrials.toString()
  }

  const startButtonStateToString = (startButtonState: StartButtonState): string => {
    switch (startButtonState) {
      case StartButtonState.Updating:
        return 'Updating...'
      case StartButtonState.Start:
        return 'Start'
      case StartButtonState.Pausing:
        return 'Pausing...'
      case StartButtonState.Pause:
        return 'Pause'
      case StartButtonState.Resume:
        return 'Resume'
    }
  }

  const cancelButtonStateToString = (cancelButtonState: CancelButtonState): string => {
    switch (cancelButtonState) {
      case CancelButtonState.Cancel:
        return 'Cancel'
      case CancelButtonState.Canceling:
        return 'Canceling...'
    }
  }

  const runStartButtonAction = (startButtonState: StartButtonState) => {
    switch (startButtonState) {
      case StartButtonState.Updating:
        break
      case StartButtonState.Start:
        perform()
        break
      case StartButtonState.Pause:
        pauseExperiment(() => {
          setStartButtonState(StartButtonState.Pausing)
        })
        break
      case StartButtonState.Resume:
        resumeExperiment(() => {
          console.log('resumed')
        })
        break
    }
  }

  const runCancelButtonAction = () => {
    cancelExperiment(() => {
      setCancelButtonState(CancelButtonState.Canceling)
    })
  }

  return (
    <>
      <ExperimentMetadata>
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

        <InputRow>
          <Label>Experiment:</Label>
          <Input
            type='text'
            placeholder='E.g., Resting motor threshold experiment 1'
            value={experimentName}
            onChange={(e) => setExperimentName(e.target.value)}
            style={{ backgroundColor: experimentName ? 'transparent' : 'lightgray' }}
          />
        </InputRow>
        <InputRow>
          <Label>Subject:</Label>
          <Input
            type='text'
            placeholder='E.g., Subject 1'
            value={subjectName}
            onChange={(e) => setSubjectName(e.target.value)}
            style={{ backgroundColor: subjectName ? 'transparent' : 'lightgray' }}
          />
        </InputRow>
      </ExperimentMetadata>

      <TabBar>
        <a
          href='#'
          onClick={() => changeActiveTab(ExperimentTab.SingleLocation)}
          className={activeTab === ExperimentTab.SingleLocation ? 'active' : ''}
        >
          Single Location
        </a>
        <a
          href='#'
          onClick={() => changeActiveTab(ExperimentTab.MultipleLocations)}
          className={activeTab === ExperimentTab.MultipleLocations ? 'active' : ''}
        >
          Multiple Locations
        </a>
      </TabBar>

      <StimulationParametersPanel>
        <GridPanel>
          {activeTab === ExperimentTab.SingleLocation && (
            <LocationSelector
              selectedPoints={selectedPoints}
              setSelectedPoints={setSelectedPoints}
              highlightedPoints={highlightedPoints}
            />
          )}
          {activeTab === ExperimentTab.MultipleLocations && (
            <LocationSelector
              selectedPoints={selectedPoints}
              setSelectedPoints={setSelectedPoints}
              highlightedPoints={highlightedPoints}
              multiSelectMode={true}
            />
          )}
        </GridPanel>
        <AnglePanel>
          {activeTab === ExperimentTab.SingleLocation && (
            <AngleSelector
              selectedAngles={selectedAngles}
              setSelectedAngles={setSelectedAngles}
              highlightedAngles={highlightedAngles}
            />
          )}
          {activeTab === ExperimentTab.MultipleLocations && (
            <AngleSelector
              selectedAngles={selectedAngles}
              setSelectedAngles={setSelectedAngles}
              highlightedAngles={highlightedAngles}
              multiSelectMode={true}
            />
          )}
        </AnglePanel>
        <IntensityPanel>
          {activeTab === ExperimentTab.SingleLocation && (
            <IntensitySelector
              value={intensity}
              min={0}
              max={150}
              showMaximumIntensity={true}
              maximumIntensity={maximumIntensity}
              onValueChange={handleIntensityChange}
            />
          )}
          {activeTab === ExperimentTab.MultipleLocations && (
            <IntensitySelector
              value={intensity}
              min={0}
              max={150}
              showMaximumIntensity={false}
              maximumIntensity={0}
              onValueChange={handleIntensityChange}
            />
          )}
        </IntensityPanel>
      </StimulationParametersPanel>
      <ConfigPanel>
        <TriggerPanel>
          <SmallerTitle>Triggers</SmallerTitle>
          <TriggerRow>
            <TriggerLabel>1</TriggerLabel>
            <ToggleSwitch type='flat' checked={trigger1Enabled} onChange={setTrigger1Enabled} />
            <GrayedOutPanel isGrayedOut={!trigger1Enabled}>
              <DelayLabel>Delay (ms)</DelayLabel>
            </GrayedOutPanel>
            <GrayedOutPanel isGrayedOut={!trigger1Enabled}>
              <ValidatedInput
                value={trigger1Delay}
                min={-99}
                max={99}
                onChange={setTrigger1Delay}
                disabled={!trigger1Enabled}
              />
            </GrayedOutPanel>
          </TriggerRow>
          <TriggerRow>
            <TriggerLabel>2</TriggerLabel>
            <ToggleSwitch type='flat' checked={trigger2Enabled} onChange={setTrigger2Enabled} />
            <GrayedOutPanel isGrayedOut={!trigger2Enabled}>
              <DelayLabel>Delay (ms)</DelayLabel>
            </GrayedOutPanel>
            <GrayedOutPanel isGrayedOut={!trigger2Enabled}>
              <ValidatedInput
                value={trigger2Delay}
                min={-99}
                max={99}
                onChange={setTrigger2Delay}
                disabled={!trigger2Enabled}
              />
            </GrayedOutPanel>
          </TriggerRow>
        </TriggerPanel>
        <MepPanel isGrayedOut={!mepHealthcheckOk}>
          <SmallerTitle>MEP analysis</SmallerTitle>
          <ConfigRow>
            <ConfigLabel>Enabled:</ConfigLabel>
            <ToggleSwitch type='flat' checked={mepEnabled} onChange={setMepEnabled} disabled={!mepHealthcheckOk} />
          </ConfigRow>
          <GrayedOutPanel isGrayedOut={!mepEnabled || !mepHealthcheckOk}>
            <ConfigRow>
              <IndentedLabel>EMG channel:</IndentedLabel>
              {/*
                  TODO: Validate EMG channel by passing a prop to ValidatedInput, based on the
                  number of available EMG channels, published in EegInfo topic by EEG bridge.
                */}
              <ValidatedInput
                type='text'
                value={emgChannel}
                min={1}
                max={16}
                onChange={setEmgChannel}
                disabled={!mepEnabled}
              />
            </ConfigRow>
          </GrayedOutPanel>
        </MepPanel>
        <TrialsPanel>
          <SmallerTitle>{activeTab === ExperimentTab.SingleLocation ? 'Trials' : 'Repetitions'}</SmallerTitle>
          <ConfigRow>
            <ConfigLabel># of {activeTab === ExperimentTab.SingleLocation ? 'trials' : 'repetitions'}:</ConfigLabel>
            <ValidatedInput type='text' value={numOfRepetitions} min={1} max={999} onChange={setNumOfRepetitions} />
          </ConfigRow>
          <ConfigRow>
            <ConfigLabel>Wait for trigger:</ConfigLabel>
            <ToggleSwitch type='flat' checked={waitForTrigger} onChange={setWaitForTrigger} />
          </ConfigRow>
          <GrayedOutPanel isGrayedOut={waitForTrigger || numOfTrials === null || numOfTrials < 2}>
            <ConfigRow>
              <ConfigLabel>Interval (s)</ConfigLabel>
            </ConfigRow>
            <CloseConfigRow>
              <IndentedLabel>Min:</IndentedLabel>
              <ValidatedInput
                type='number'
                value={itiMin}
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
                type='number'
                value={itiMax}
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
            <ToggleSwitch type='flat' checked={autopause} onChange={setAutopause} />
          </ConfigRow>
          <GrayedOutPanel isGrayedOut={!autopause}>
            <ConfigRow>
              <IndentedLabel>Interval (min):</IndentedLabel>
              <ValidatedInput
                type='number'
                value={autopauseIntervalMinutes}
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
            <ConfigLabel>{duration ? formatTime(duration) : '\u2013'}</ConfigLabel>
          </CloseConfigRow>
        </ExperimentPanel>
        <StatusPanel>
          <SmallerTitle>Status</SmallerTitle>
          <ConfigRow>
            <ConfigLabel>Experiment</ConfigLabel>
            <ConfigLabel>{formatExperimentState()} </ConfigLabel>
          </ConfigRow>
          <ConfigRow>
            <IndentedLabel>Trial:</IndentedLabel>
            <ConfigLabel>{formatTrialNumber()} </ConfigLabel>
          </ConfigRow>
          <CloseConfigRow>
            <IndentedLabel>Attempt:</IndentedLabel>
            <ConfigLabel>{attemptNumber !== null ? attemptNumber : '\u2013'}</ConfigLabel>
          </CloseConfigRow>
          <CloseConfigRow></CloseConfigRow>
          <ConfigRow>
            <ConfigLabel>Time:</ConfigLabel>
            <ConfigLabel>
              {experimentState === ExperimentState.Running || experimentState === ExperimentState.Paused
                ? formatTime(systemState?.time)
                : '\u2013'}
            </ConfigLabel>
          </ConfigRow>
          <CloseConfigRow></CloseConfigRow>
          <StyledButton
            onClick={() => runStartButtonAction(startButtonState)}
            disabled={
              startButtonState === StartButtonState.Updating ||
              startButtonState === StartButtonState.Pausing ||
              selectedPoints.length == 0 ||
              selectedAngles.length == 0
            }
          >
            {startButtonStateToString(startButtonState)}
          </StyledButton>
          <StyledRedButton
            onClick={() => runCancelButtonAction()}
            disabled={
              experimentState === ExperimentState.NotRunning || cancelButtonState == CancelButtonState.Canceling
            }
          >
            {cancelButtonStateToString(cancelButtonState)}
          </StyledRedButton>
        </StatusPanel>
      </ConfigPanel>
    </>
  )
}
