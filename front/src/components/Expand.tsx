import React from 'react'
import { ReactComponent as ArrowDown } from 'assets/arrow-down.svg'
import { ReactComponent as ArrowLeft } from 'assets/arrow-left.svg'

import styled from 'styled-components'

interface Props {
  expanded: boolean
  onClick: (event: any) => any
}

const Expand = ({ expanded, onClick }: Props) => {
  return <ExpandButton onClick={onClick}>{expanded ? <StyledArrowDown /> : <StyledArrowLeft />}</ExpandButton>
}

const ExpandButton = styled.button`
  all: unset;
`

const StyledArrowDown = styled(ArrowDown)`
  width: 20px;
  height: 20px;
  vertical-align: center;
  text-align: center;
`

const StyledArrowLeft = styled(ArrowLeft)`
  width: 20px;
  height: 20px;
  vertical-align: center;
  text-align: center;
`

export default Expand
