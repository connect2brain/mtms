import React from 'react'
import styled from 'styled-components'

import { SystemState } from 'components/system/SystemState'
import { DeviceControl } from 'components/system/DeviceControl'

export const SystemView = () => {
  return (
    <>
      <Wrapper>
        <LeftPanel>
          <DeviceControl />
        </LeftPanel>
        <RightPanel>
          <SystemState />
        </RightPanel>
      </Wrapper>
    </>
  )
}

const Wrapper = styled.div`
  display: grid;
  grid-template-columns: minmax(20rem, 26rem) minmax(24rem, 38rem);
  grid-template-rows: auto;
  gap: 1rem;
  padding: 1rem;
`

const styledPanel = `
  padding: 1rem;
  border-radius: 5px;
  background-color: #f7f7f7;
  box-shadow: 0px 3px 6px rgba(0, 0, 0, 0.1);/
`

const LeftPanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 1 / 2;
  ${styledPanel}
`

const RightPanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 2 / 3;
  display: flex;
  flex-direction: column;
  gap: 1.5rem;
  ${styledPanel}
`
