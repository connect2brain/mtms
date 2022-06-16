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

const initialState: SequenceState = {
  sequences: [],
  expandedSequences: {},
}

const sequenceSlice = createSlice({
  name: 'sequences',
  initialState,
  reducers: {
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
  },
})

export const { addSequence, modifySequence, setSequences, setExpandedSequences, setExpandedSequence } =
  sequenceSlice.actions
export default sequenceSlice.reducer
