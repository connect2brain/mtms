import React from 'react'
import { ThemeProvider } from 'styled-components'
import theme from 'styles/theme'

import { RosConnectionProvider } from './RosConnectionProvider'
import { HeartbeatProvider } from './HeartbeatProvider'
import { SystemProvider } from './SystemProvider'
import { ConfigProvider } from './ConfigProvider'
import { HealthProvider } from './HealthProvider'
import { TrialProvider } from './TrialProvider'
import { EegDeviceInfoProvider } from './EegDeviceInfoProvider'
import { ExperimentProvider } from './ExperimentProvider'
import { RemoteControllerProvider } from './RemoteControllerProvider'

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
              <HealthProvider>
                <RemoteControllerProvider>
                <TrialProvider>
                  <EegDeviceInfoProvider>
                    <ExperimentProvider>{children}</ExperimentProvider>
                  </EegDeviceInfoProvider>
                </TrialProvider>
                </RemoteControllerProvider>
              </HealthProvider>
            </ConfigProvider>
          </SystemProvider>
        </HeartbeatProvider>
      </RosConnectionProvider>
    </ThemeProvider>
  )
}

export default Providers
