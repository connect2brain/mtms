import React, { useEffect, useState } from 'react'
import styled from 'styled-components'

import { GridComponent } from 'components/GridComponent'
import { GridControls } from 'components/GridControls'

const Wrapper = styled.div`
  display: grid;
  grid-template-columns: 1fr 1fr;
  grid-template-rows: auto 1fr;
  gap: 1rem;
  padding: 1rem;
`

const styledPanel = `
  padding: 25px 0px 40px 35px;
  border-radius: 5px;
  background-color: #f7f7f7;
  box-shadow: 0px 3px 6px rgba(0, 0, 0, 0.1);/
`

const PanelA = styled.div`
  grid-row: 1 / 2;
  grid-column: 1 / 2;
  ${styledPanel}
`

export const ExperimentControl = () => {
  return (
    <Wrapper>
      <PanelA>
        <GridComponent multiSelectMode={false} />
      </PanelA>
    </Wrapper>
  )
}
