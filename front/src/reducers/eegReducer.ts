import { createSlice, PayloadAction } from '@reduxjs/toolkit'
import { EegTrigger, EegDatapoint, EegBatchMessage, EegBatch } from 'types/eeg'

interface EegState {
  eeg: EegDatapoint[]
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
    setEeg: (state, action: PayloadAction<EegDatapoint[]>) => {
      state.eeg = action.payload
      //state.eeg.splice(0, state.maxLength - state.eeg.length)
    },
    addBatch: (state, action: PayloadAction<EegDatapoint[]>) => {
      state.eeg.push(...action.payload)
      state.eeg.splice(0, Math.max(0, state.eeg.length - state.maxLength))
    },
    addEegDatapoint: (state, action: PayloadAction<EegDatapoint>) => {
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

export const { setEeg, setMaxLength, addEegTrigger, addEegDatapoint, addBatch } = eegSlice.actions
export default eegSlice.reducer
