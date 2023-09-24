import React, { useEffect, useState } from 'react'
import styled from 'styled-components'

import { nodeMessageSubscriber } from 'services/ros'

type MessageWithTimestamp = {
  message: string
  timestamp: Date
}

export const NodeState: React.FC = () => {
  const locale = process.env.REACT_APP_LOCALE || 'en-US'

  const [messages, setMessages] = useState<MessageWithTimestamp[]>([])

  const removeMessage = (targetMsg: MessageWithTimestamp) => {
    setMessages(prev => prev.filter(msg => msg !== targetMsg))
  }

  useEffect(() => {
    const handleNewMessage = (message: any) => {
      const newMessageWithTimestamp: MessageWithTimestamp = {
        message: message.data,
        timestamp: new Date()
      }
      setMessages(prev => [...prev, newMessageWithTimestamp])

      setTimeout(() => {
        removeMessage(newMessageWithTimestamp)
      }, 5000)
    }
    nodeMessageSubscriber.subscribe(handleNewMessage)

    return () => {
      nodeMessageSubscriber.unsubscribe(handleNewMessage)
    }
  }, [])

  const lastMessages = messages.slice(-3)

  return (
    <MessagesContainer>
      <Header>Messages</Header>
      {[...lastMessages].reverse().map((msg, index) => (
        <Message key={index} style={{ opacity: (3 - index) / 3 }}>
          <span>{msg.timestamp.toLocaleTimeString(locale)}</span> &#8212; {msg.message}
        </Message>
      ))}
    </MessagesContainer>
  )
}

const MessagesContainer = styled.div`
  height: 110px;
  overflow-y: auto;
`

const Header = styled.div`
  color: #333;
  font-weight: bold;
  font-size: 1.1rem;
  margin-bottom: 0.5rem;
`

const Message = styled.div`
  color: #333;
  font-weight: bold;
  font-size: 0.9rem;
  padding: 5px;
  border-bottom: 0px;
  transition: opacity 0.3s;
`
