import { configureStore } from '@reduxjs/toolkit'
import targetReducer from 'reducers/planner/targetReducer'
import sequenceReducer from 'reducers/planner/sequenceReducer'
import eegReducer from 'reducers/visualizer/eegReducer'

export const store = configureStore({
  reducer: {
    targets: targetReducer,
    sequences: sequenceReducer,
    eegData: eegReducer,
  },
  middleware: (getDefaultMiddleware) => {
    return getDefaultMiddleware({
      serializableCheck: false,
    })
  },
})

// Infer the `RootState` and `AppDispatch` types from the store itself
export type RootState = ReturnType<typeof store.getState>
// Inferred type: {posts: PostsState, comments: CommentsState, users: UsersState}
export type AppDispatch = typeof store.dispatch
