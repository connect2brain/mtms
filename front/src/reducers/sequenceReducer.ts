import { createSlice, PayloadAction } from '@reduxjs/toolkit'
import { PulseSequence } from 'types/pulseSequence'

interface SequenceState {
  sequences: PulseSequence[]
  expandedSequences: Record<string, boolean>
}

interface ModifyPulseSequencePayload {
  index: number
  pulseSequence: PulseSequence
}

interface SetExpandedSequencePayload {
  index: number
  expanded: boolean
}

interface RemovePulsesFromSequencePayload {
  sequenceIndex: number
  pulseIdsToRemove: number[]
}

const initialState: SequenceState = {
  sequences: [],
  expandedSequences: {},
}

const sequenceSlice = createSlice({
  name: 'sequences',
  initialState,
  reducers: {
    setPulseSequences: (state, action: PayloadAction<PulseSequence[]>) => {
      state.sequences = action.payload
    },
    addSequence: (state, action: PayloadAction<PulseSequence>) => {
      state.sequences.push(action.payload)
      console.log(`Created new sequence ${action.payload.name} with ${action.payload.pulses.length} targets`)
    },
    setSequences: (state, action: PayloadAction<PulseSequence[]>) => {
      state.sequences = action.payload
    },
    modifySequence: (state, action: PayloadAction<ModifyPulseSequencePayload>) => {
      state.sequences[action.payload.index] = action.payload.pulseSequence
    },
    setExpandedSequences: (state, action: PayloadAction<Record<string, boolean>>) => {
      state.expandedSequences = action.payload
    },
    setExpandedSequence: (state, action: PayloadAction<SetExpandedSequencePayload>) => {
      state.expandedSequences[action.payload.index] = action.payload.expanded
    },
    removePulsesFromSequence: (state, action: PayloadAction<RemovePulsesFromSequencePayload>) => {
      const sequence = state.sequences[action.payload.sequenceIndex]
      const pulseIdsToRemove = action.payload.pulseIdsToRemove
      const targetIdsToRemove = pulseIdsToRemove.map((id) => sequence.pulses[id].targetIndex)
      let updatedPulses = sequence.pulses.filter((pulse, i) => !pulseIdsToRemove.includes(i))
      targetIdsToRemove.forEach((id) => {
        updatedPulses = updatedPulses.map((pulse) => {
          const newIndex = pulse.targetIndex < id ? pulse.targetIndex : pulse.targetIndex - 1
          return {
            ...pulse,
            targetIndex: newIndex,
          }
        })
      })

      state.sequences[action.payload.sequenceIndex] = {
        ...sequence,
        pulses: updatedPulses,
      }
    },
  },
})

export const {
  addSequence,
  modifySequence,
  setSequences,
  removePulsesFromSequence,
  setExpandedSequence,
  setPulseSequences,
} = sequenceSlice.actions
export default sequenceSlice.reducer
