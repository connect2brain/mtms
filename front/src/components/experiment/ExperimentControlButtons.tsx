import React, { useContext, useEffect, useState } from 'react'

import { StyledButton, StyledRedButton } from 'styles/General'
import { cancelExperiment, pauseExperiment, resumeExperiment } from 'ros/experiment'
import { EXPERIMENT_STATE, ExperimentContext } from 'providers/ExperimentProvider'

const START_DISABLED_INERTIA_MS = 200

export const ExperimentControlButtons: React.FC<{
  onStart: () => void
  startDisabled: boolean
}> = ({ onStart, startDisabled }) => {
  const { experimentStateMessage } = useContext(ExperimentContext)
  const experimentState = experimentStateMessage?.state

  const [smoothStartDisabled, setSmoothStartDisabled] = useState(startDisabled)

  useEffect(() => {
    const id = window.setTimeout(() => {
      setSmoothStartDisabled(startDisabled)
    }, START_DISABLED_INERTIA_MS)
    return () => window.clearTimeout(id)
  }, [startDisabled])

  const cancelEnabled =
    experimentState === EXPERIMENT_STATE.RUNNING ||
    experimentState === EXPERIMENT_STATE.PAUSE_REQUESTED ||
    experimentState === EXPERIMENT_STATE.PAUSED

  let primaryLabel = 'Start'
  let primaryDisabled = smoothStartDisabled
  let runPrimary = () => onStart()

  if (experimentState === EXPERIMENT_STATE.STOPPING) {
    primaryLabel = 'Stopping...'
    primaryDisabled = true
    runPrimary = () => undefined
  } else if (experimentState === EXPERIMENT_STATE.CANCEL_REQUESTED) {
    primaryLabel = 'Canceling...'
    primaryDisabled = true
    runPrimary = () => undefined
  } else if (experimentState === EXPERIMENT_STATE.PAUSE_REQUESTED) {
    primaryLabel = 'Pausing...'
    primaryDisabled = true
    runPrimary = () => undefined
  } else if (experimentState === EXPERIMENT_STATE.RUNNING) {
    primaryLabel = 'Pause'
    primaryDisabled = false
    runPrimary = () => pauseExperiment(() => undefined)
  } else if (experimentState === EXPERIMENT_STATE.PAUSED) {
    primaryLabel = 'Resume'
    primaryDisabled = false
    runPrimary = () => resumeExperiment(() => undefined)
  }

  return (
    <>
      <StyledButton onClick={runPrimary} disabled={primaryDisabled}>
        {primaryLabel}
      </StyledButton>
      <StyledRedButton onClick={() => cancelExperiment()} disabled={!cancelEnabled}>
        Cancel
      </StyledRedButton>
    </>
  )
}
