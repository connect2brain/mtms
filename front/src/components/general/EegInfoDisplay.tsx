import React, { useContext } from 'react'
import styled from 'styled-components'

import { EegDeviceInfoContext } from 'providers/EegDeviceInfoProvider'
import { StyledPanel } from 'styles/General'

const EegInfoPanel = styled(StyledPanel)<{ isGrayedOut: boolean }>`
  width: 250px;
  position: fixed;
  top: 230px;
  right: 5px;
  z-index: 1000;
  padding: 15px 20px;

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

const EegHeader = styled.div<{ isGrayedOut: boolean }>`
  color: ${({ isGrayedOut }) => (isGrayedOut ? '#959595' : '#333')};
  font-weight: bold;
  font-size: 0.9rem;
  margin-bottom: 0.5rem;
  position: fixed;
  top: 210px;
  right: 270px;
  z-index: 1001;
  transition: color 0.3s ease;
`

const Header = styled.div`
  color: #333;
  font-weight: bold;
  font-size: 1.1rem;
  margin-bottom: 0.5rem;
`

const InfoRow = styled.div`
  display: flex;
  justify-content: space-between;
  gap: 1rem;
  margin-top: 0.35rem;
  font-size: 0.9rem;
`

const ChannelSection = styled.div`
  margin-top: 0.5rem;
`

const ChannelLabel = styled.div`
  font-weight: 600;
  font-size: 0.85rem;
  margin-bottom: 0.2rem;
  color: #666;
`

const ChannelRow = styled.div`
  display: flex;
  justify-content: space-between;
  gap: 1rem;
  margin-left: 0.5rem;
  font-size: 0.85rem;
`

const InfoLabel = styled.span`
  font-weight: 400;
`

const InfoValue = styled.span`
  font-weight: 600;
`

export const EegInfoDisplay: React.FC = () => {
  const { eegDeviceInfo } = useContext(EegDeviceInfoContext)

  const isStreaming = Boolean(eegDeviceInfo?.is_streaming)
  const samplingFrequencyValue = isStreaming
    ? `${eegDeviceInfo?.sampling_frequency} Hz`
    : '\u2013'
  const eegChannelsValue = isStreaming ? eegDeviceInfo?.num_eeg_channels : '\u2013'
  const emgChannelsValue = isStreaming ? eegDeviceInfo?.num_emg_channels : '\u2013'

  return (
    <>
      <EegHeader isGrayedOut={!isStreaming}>EEG</EegHeader>
      <EegInfoPanel isGrayedOut={!isStreaming}>
        <InfoRow>
          <ChannelLabel>Sampling frequency:</ChannelLabel>
          <InfoValue>{samplingFrequencyValue}</InfoValue>
        </InfoRow>
        <ChannelSection>
          <ChannelLabel>Channels:</ChannelLabel>
          <ChannelRow>
            <ChannelLabel>EEG</ChannelLabel>
            <InfoValue>{eegChannelsValue}</InfoValue>
          </ChannelRow>
          <ChannelRow>
            <ChannelLabel>EMG</ChannelLabel>
            <InfoValue>{emgChannelsValue}</InfoValue>
          </ChannelRow>
        </ChannelSection>
      </EegInfoPanel>
    </>
  )
}
