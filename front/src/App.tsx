import React from 'react'
import './App.css'
import Providers from './providers/Providers'
import styled from 'styled-components'
import Targets from 'views/Targets';

function App() {
  return (
    <Providers>
      <Wrapper>
        <Header>mTMS control panel</Header>
        <Targets />
      </Wrapper>
    </Providers>
  )
}

const Wrapper = styled.div`
  padding: 0.5rem;
`

const Header = styled.h1`
  color: ${(p) => p.theme.colors.primary};
`

export default App
