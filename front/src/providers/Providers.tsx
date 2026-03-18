import React from 'react'
import { ThemeProvider } from 'styled-components'
import theme from 'styles/theme'

import { SystemProvider } from './SystemProvider'
import { ConfigProvider } from './ConfigProvider'
import { HealthcheckProvider } from './HealthcheckProvider'

interface Props {
  children: React.ReactNode
}

const Providers: React.FC<Props> = ({ children }) => {
  return (
    <ThemeProvider theme={theme}>
      <SystemProvider>
        <ConfigProvider>
          <HealthcheckProvider>
            {children}
          </HealthcheckProvider>
        </ConfigProvider>
      </SystemProvider>
    </ThemeProvider>
  )
}

export default Providers
