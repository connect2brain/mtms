import { createSlice, PayloadAction } from '@reduxjs/toolkit'
import { EegTrigger, EegSample, EegBatchMessage, EegBatch } from 'types/eeg'

interface EegState {
  eeg: EegSample[]
  triggers: EegTrigger[]
  maxLength: number
}

const initialState: EegState = {
  eeg: [],
  triggers: [],
  maxLength: 1000,
}

const eegSlice = createSlice({
  name: 'eeg',
  initialState,
  reducers: {
    setEeg: (state, action: PayloadAction<EegSample[]>) => {
      state.eeg = action.payload
    },
    addBatch: (state, action: PayloadAction<EegSample[]>) => {
      state.eeg.push(...action.payload)
      state.eeg.splice(0, Math.max(0, state.eeg.length - state.maxLength))
    },
    addEegSample: (state, action: PayloadAction<EegSample>) => {
      state.eeg.push(action.payload)
      if (state.eeg.length > state.maxLength) {
        state.eeg.shift()
      }
    },
    addEegTrigger: (state, action: PayloadAction<EegTrigger>) => {
      state.triggers.push(action.payload)
      if (state.triggers.length > state.maxLength) {
        state.triggers.shift()
      }
    },
    setMaxLength: (state, action: PayloadAction<number>) => {
      state.maxLength = action.payload
    },
  },
})

export const { setEeg, setMaxLength, addEegTrigger, addEegSample, addBatch } = eegSlice.actions
export default eegSlice.reducer
