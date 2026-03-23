import React from 'react'
import { ThemeProvider } from 'styled-components'
import theme from 'styles/theme'

import { RosConnectionProvider } from './RosConnectionProvider'
import { HeartbeatProvider } from './HeartbeatProvider'
import { SystemProvider } from './SystemProvider'
import { ConfigProvider } from './ConfigProvider'
import { HealthcheckProvider } from './HealthcheckProvider'
import { TrialProvider } from './TrialProvider'
import { EegDeviceInfoProvider } from './EegDeviceInfoProvider'
import { ExperimentProvider } from './ExperimentProvider'

interface Props {
  children: React.ReactNode
}

const Providers: React.FC<Props> = ({ children }) => {
  return (
    <ThemeProvider theme={theme}>
      <RosConnectionProvider>
        <HeartbeatProvider>
          <SystemProvider>
            <ConfigProvider>
              <HealthcheckProvider>
                <TrialProvider>
                  <EegDeviceInfoProvider>
                    <ExperimentProvider>{children}</ExperimentProvider>
                  </EegDeviceInfoProvider>
                </TrialProvider>
              </HealthcheckProvider>
            </ConfigProvider>
          </SystemProvider>
        </HeartbeatProvider>
      </RosConnectionProvider>
    </ThemeProvider>
  )
}

export default Providers
