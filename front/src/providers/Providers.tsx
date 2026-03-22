import React from 'react'
import { ThemeProvider } from 'styled-components'
import theme from 'styles/theme'

import { RosConnectionProvider } from './RosConnectionProvider'
import { SystemProvider } from './SystemProvider'
import { ConfigProvider } from './ConfigProvider'
import { HealthcheckProvider } from './HealthcheckProvider'
import { EegDeviceInfoProvider } from './EegDeviceInfoProvider'
import { ExperimentProvider } from './ExperimentProvider'

interface Props {
  children: React.ReactNode
}

const Providers: React.FC<Props> = ({ children }) => {
  return (
    <ThemeProvider theme={theme}>
      <RosConnectionProvider>
        <SystemProvider>
          <ConfigProvider>
            <HealthcheckProvider>
              <EegDeviceInfoProvider>
                <ExperimentProvider>{children}</ExperimentProvider>
              </EegDeviceInfoProvider>
            </HealthcheckProvider>
          </ConfigProvider>
        </SystemProvider>
      </RosConnectionProvider>
    </ThemeProvider>
  )
}

export default Providers
