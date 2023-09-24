import React, { useEffect, useState } from 'react'
import styled from 'styled-components'

import { nodeMessageSubscriber } from 'services/ros'

export const NodeState: React.FC = () => {
  const [messages, setMessages] = useState<string[]>([])

  useEffect(() => {
    const handleNewMessage = (message: any) => {
      setMessages(prev => [...prev, message.data])
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
      {[...lastMessages].reverse().map((message, index) => (
          <Message key={index} style={{ opacity: (3 - index) / 3 }}>
              {message}
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
