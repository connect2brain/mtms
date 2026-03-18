import React from 'react'
import { ThemeProvider } from 'styled-components'
import theme from 'styles/theme'

import { SystemProvider } from './SystemProvider'
import { ConfigProvider } from './ConfigProvider'
import { HealthcheckProvider } from './HealthcheckProvider'
import { EegDeviceInfoProvider } from './EegDeviceInfoProvider'

interface Props {
  children: React.ReactNode
}

const Providers: React.FC<Props> = ({ children }) => {
  return (
    <ThemeProvider theme={theme}>
      <SystemProvider>
        <ConfigProvider>
          <HealthcheckProvider>
            <EegDeviceInfoProvider>{children}</EegDeviceInfoProvider>
          </HealthcheckProvider>
        </ConfigProvider>
      </SystemProvider>
    </ThemeProvider>
  )
}

export default Providers
