import React from 'react'
import styled from 'styled-components'
import { useNavigate } from 'react-router-dom'

export const Navbar = () => {
  const navigate = useNavigate()

  const goToUrl = (url: string) => {
    navigate(url, { replace: true })
  }

  return (
    <Nav>
      <Ul>
        <ListElement onClick={() => goToUrl('/')}>Targets table</ListElement>
        <ListElement onClick={() => goToUrl('/experiment')}>Experiment</ListElement>
        <ListElement onClick={() => goToUrl('/plot')}>Plot</ListElement>
      </Ul>
    </Nav>
  )
}

const Nav = styled.nav`
  margin: 0;
  padding-left: 0.5em;
  box-sizing: border-box;
`

const Ul = styled.ul`
  list-style-type: none;
  margin: 0;
  padding: 0;
  display: flex;
  flex-direction: row;
`

const ListElement = styled.li`
  cursor: pointer;
  font-size: ${p => p.theme.typography.medium};
  padding-right: 0.5em;
  padding-left: 0.5em;
  box-shadow: inset 0 -1px 0 ${(p) => p.theme.colors.gray};
  
  background-color: ${(p) => p.theme.colors.lightergray};
  
  :hover {
    background-color: ${(p) => p.theme.colors.lightgray};
  }
`
