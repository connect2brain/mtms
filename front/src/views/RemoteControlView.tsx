import React from 'react'
import styled from 'styled-components'

import { RemoteControl } from 'components/remote_control/RemoteControl'

export const RemoteControlView = () => {
  return (
    <RemoteControlPage>
      <RemoteControl />
    </RemoteControlPage>
  )
}

const RemoteControlPage = styled.div`
  width: 600px;
  height: 550px;
  display: flex;
  flex-direction: column;
  align-items: flex-start;
`

