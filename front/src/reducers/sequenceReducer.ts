import { createSlice, PayloadAction } from '@reduxjs/toolkit'
import { PulseSequence } from 'types/pulseSequence'

interface SequenceState {
  sequences: PulseSequence[]
}

interface ModifyPulseSequencePayload {
  index: number
  pulseSequence: PulseSequence
}

const initialState: SequenceState = {
  sequences: [],
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
  },
})

export const { addSequence, modifySequence, setSequences } = sequenceSlice.actions
export default sequenceSlice.reducer
