import React, { useContext, useMemo } from 'react'
import styled from 'styled-components'

import { SystemContext, DeviceState } from 'providers/SystemProvider'
import { StyledPanel } from 'styles/General'

const VoltagePanel = styled(StyledPanel)<{ isGrayedOut: boolean }>`
  width: 250px;
  position: fixed;
  top: 200px;
  right: 5px;
  z-index: 1000;
  padding: 15px 20px;
  max-height: 280px;
  overflow-y: auto;

  ${({ isGrayedOut }) =>
    isGrayedOut &&
    `
    background-color: #ebebeb;
    filter: grayscale(100%);
    opacity: 0.78;
    box-shadow: 0px 2px 4px rgba(0, 0, 0, 0.06);

    &, & * {
      color: #959595 !important;
    }
  `}
`

const VoltageTable = styled.table`
  width: 100%;
  border-collapse: collapse;
  font-size: 0.9rem;
  line-height: 1.35;
`

const TableHead = styled.thead`
  border-bottom: 1px solid #d8d8d8;
`

const ThCol = styled.th`
  padding: 0 0 0.5rem;
  text-align: left;
  font-weight: 600;
  font-size: 0.72rem;
  letter-spacing: 0.04em;
  text-transform: uppercase;
  color: #666;
`

const ThVoltage = styled(ThCol)`
  text-align: right;
  padding-left: 1rem;
`

const TdIndex = styled.td`
  padding: 0.4rem 0 0;
  font-weight: 500;
  font-variant-numeric: tabular-nums;
`

const TdVolts = styled.td`
  padding: 0.4rem 0 0;
  text-align: right;
  font-weight: 600;
  font-variant-numeric: tabular-nums;
`

const TdPlaceholder = styled.td`
  padding: 0.5rem 0 0;
  text-align: center;
  color: #888;
  font-size: 0.85rem;
`

function formatVoltage(v: number | undefined): string {
  if (v === undefined || v === null || Number.isNaN(v)) {
    return '\u2013'
  }
  return v.toFixed(0)
}

export const ChannelVoltageDisplay: React.FC = () => {
  const { systemState } = useContext(SystemContext)

  const isDeviceOn =
    systemState?.device_state?.value !== undefined &&
    systemState.device_state.value !== DeviceState.NOT_OPERATIONAL

  const rows = useMemo(() => {
    if (!systemState?.channel_states?.length) {
      return []
    }
    return [...systemState.channel_states].sort((a, b) => a.channel_index - b.channel_index)
  }, [systemState?.channel_states])

  return (
    <>
      <VoltagePanel isGrayedOut={!isDeviceOn}>
        <VoltageTable>
          <TableHead>
            <tr>
              <ThCol scope="col">Channel</ThCol>
              <ThVoltage scope="col">Voltage</ThVoltage>
            </tr>
          </TableHead>
          <tbody>
            {rows.length === 0 ? (
              <tr>
                <TdPlaceholder colSpan={2}>{'\u2013'}</TdPlaceholder>
              </tr>
            ) : (
              rows.map((ch) => (
                <tr key={ch.channel_index}>
                  <TdIndex>{ch.channel_index + 1}</TdIndex>
                  <TdVolts>{formatVoltage(ch.voltage)}</TdVolts>
                </tr>
              ))
            )}
          </tbody>
        </VoltageTable>
      </VoltagePanel>
    </>
  )
}

