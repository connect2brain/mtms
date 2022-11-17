import React, { useEffect, useState } from 'react'
import styled from 'styled-components'
import { Feedback } from 'types/fpga'
import { getKeyByValue } from 'utils'
import { errorsByType } from '../types/fpgaErrors'

type Props = {
  feedback: Feedback | undefined
}

export const EventFeedbacks = ({ feedback }: Props) => {
  const [successfulEvents, setSuccessfulEvents] = useState<Feedback[]>([])
  const [failedEvents, setFailedEvents] = useState<Feedback[]>([])

  useEffect(() => {
    if (!feedback) return

    if (feedback.error.value == 0) {
      setSuccessfulEvents([...successfulEvents, feedback])
    } else {
      setFailedEvents([...failedEvents, feedback])
    }
  }, [feedback])

  return (
    <div>
      <h3>Event feedbacks</h3>
      <h4>Successful events:</h4>
      {successfulEvents.map((successfulEvent) => {
        return <p key={`successful-event-${successfulEvent.id}`}>Event successful: {successfulEvent.id}</p>
      })}
      <h4>Failed events:</h4>
      {failedEvents.map((failedEvent) => {
        return (
          <p key={`failed-event-${failedEvent.id}`}>
            Event failed: {failedEvent.id}, reason:{' '}
            {getKeyByValue(errorsByType[failedEvent.type], failedEvent.error.value)}
          </p>
        )
      })}
    </div>
  )
}
