import React, { useEffect } from 'react'
import { eegDataSubscriber } from 'services/ros'
import { EegDatapoint, EegDatapointMessage } from 'types/eeg'
import { useAppDispatch, useAppSelector } from 'providers/reduxHooks'
import { addEegDatapoint } from 'reducers/eegReducer'
import { EegChart } from 'components/EegChart'

const DataVisualize = () => {
  const { eeg } = useAppSelector((state) => state.eegData)

  const dispatch = useAppDispatch()

  useEffect(() => {
    console.log('Subscribing to eeg data')
    eegDataSubscriber.subscribe(newEegDatapoint)
  }, [])

  const newEegDatapoint = (message: EegDatapointMessage) => {
    const datapoint: EegDatapoint = {
      channel_datapoint: message.channel_datapoint,
      time: message.time,
      first_sample_of_experiment: message.first_sample_of_experiment,
    }
    //console.log('new data point', datapoint)
    dispatch(addEegDatapoint(datapoint))
  }

  const c3 = () => {
    return eeg.map((datapoint) => datapoint.channel_datapoint[4])
  }

  return (
    <div>
      Eeg length: {eeg.length}
      <EegChart data={c3()} />
    </div>
  )
}

export default DataVisualize
