import React from 'react'
import './App.css'
import Providers from './providers/Providers'
import styled from 'styled-components';

function App() {
  return (
    <Providers>
      <Header>Running</Header>
    </Providers>
  )
}

const Header = styled.h1`
  color: ${p => p.theme.colors.red};
`;

export default App
