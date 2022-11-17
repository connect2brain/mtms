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
      <EventContainer>
        {successfulEvents.map((successfulEvent) => {
          return <p key={`successful-event-${successfulEvent.id}`}>ID: {successfulEvent.id}</p>
        })}
      </EventContainer>
      <h4>Failed events:</h4>
      <EventContainer>
        {failedEvents.map((failedEvent) => {
          return (
            <p key={`failed-event-${failedEvent.id}`}>
              ID: {failedEvent.id}, reason: {getKeyByValue(errorsByType[failedEvent.type], failedEvent.error.value)}
            </p>
          )
        })}
      </EventContainer>
    </div>
  )
}

const EventContainer = styled.div`
  max-height: 400px;
  overflow-y: auto;
  overflow-x: hidden;
`
