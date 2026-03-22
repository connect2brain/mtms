import React, { useContext } from 'react'
import styled from 'styled-components'

import { EegDeviceInfoContext } from 'providers/EegDeviceInfoProvider'
import { StyledPanel } from 'styles/General'

const EegInfoPanel = styled(StyledPanel)`
  width: 250px;
  position: fixed;
  top: 395px;
  right: 5px;
  z-index: 1000;
  padding: 15px 20px;
`

const Header = styled.div`
  color: #333;
  font-weight: bold;
  font-size: 1.1rem;
  margin-bottom: 0.5rem;
`

const InfoRow = styled.div<{ $muted?: boolean }>`
  display: flex;
  justify-content: space-between;
  gap: 1rem;
  margin-top: 0.35rem;
  font-size: 0.9rem;
  color: ${({ $muted }) => ($muted ? '#959595' : undefined)};
`

const StreamingRow = styled(InfoRow)`
  margin-top: 0;
  margin-bottom: 0.65rem;
`

const ChannelSection = styled.div`
  margin-top: 0.5rem;
`

const ChannelLabel = styled.div<{ $muted?: boolean }>`
  font-weight: 600;
  font-size: 0.85rem;
  margin-bottom: 0.2rem;
  color: ${({ $muted }) => ($muted ? '#959595' : '#666')};
`

const ChannelRow = styled.div<{ $muted?: boolean }>`
  display: flex;
  justify-content: space-between;
  gap: 1rem;
  margin-left: 0.5rem;
  font-size: 0.85rem;
  color: ${({ $muted }) => ($muted ? '#959595' : undefined)};
`

const InfoLabel = styled.span`
  font-weight: 400;
`

const InfoValue = styled.span<{ $muted?: boolean }>`
  font-weight: 600;
  color: ${({ $muted }) => ($muted ? '#959595' : undefined)};
`

export const EegInfoDisplay: React.FC = () => {
  const { eegDeviceInfo } = useContext(EegDeviceInfoContext)

  const isStreaming = Boolean(eegDeviceInfo?.is_streaming)
  const samplingFrequencyValue = isStreaming
    ? `${eegDeviceInfo?.sampling_frequency} Hz`
    : '\u2013'
  const eegChannelsValue = isStreaming ? eegDeviceInfo?.num_eeg_channels : '\u2013'
  const emgChannelsValue = isStreaming ? eegDeviceInfo?.num_emg_channels : '\u2013'

  const muted = !isStreaming

  return (
    <>
      <EegInfoPanel>
        <StreamingRow>
          <ChannelLabel>EEG streaming:</ChannelLabel>
          <InfoValue>{isStreaming ? '\u2714' : '\u2717'}</InfoValue>
        </StreamingRow>
        <InfoRow $muted={muted}>
          <ChannelLabel $muted={muted}>Sampling frequency:</ChannelLabel>
          <InfoValue $muted={muted}>{samplingFrequencyValue}</InfoValue>
        </InfoRow>
        <ChannelSection>
          <ChannelLabel $muted={muted}>Channels:</ChannelLabel>
          <ChannelRow $muted={muted}>
            <ChannelLabel $muted={muted}>EEG</ChannelLabel>
            <InfoValue $muted={muted}>{eegChannelsValue}</InfoValue>
          </ChannelRow>
          <ChannelRow $muted={muted}>
            <ChannelLabel $muted={muted}>EMG</ChannelLabel>
            <InfoValue $muted={muted}>{emgChannelsValue}</InfoValue>
          </ChannelRow>
        </ChannelSection>
      </EegInfoPanel>
    </>
  )
}
