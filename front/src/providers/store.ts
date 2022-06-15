import { configureStore } from '@reduxjs/toolkit'
import targetReducer from '../reducers/targetReducer'

export const store = configureStore({
  reducer: {
    targetReducer,
  },
})

// Infer the `RootState` and `AppDispatch` types from the store itself
export type RootState = ReturnType<typeof store.getState>
// Inferred type: {posts: PostsState, comments: CommentsState, users: UsersState}
export type AppDispatch = typeof store.dispatch
