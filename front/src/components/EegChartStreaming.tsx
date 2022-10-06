import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  BarElement,
  ChartOptions,
  ChartData,
} from 'chart.js'
import 'chartjs-adapter-date-fns'
import StreamingPlugin from 'chartjs-plugin-streaming'

import { Line, Chart } from 'react-chartjs-2'
import React, { useEffect, useRef, useState } from 'react'

const options: ChartOptions<'line' | 'bar'> = {
  responsive: true,
  animation: {
    duration: 0,
  },
  plugins: {
    legend: {
      position: 'top' as const,
    },
    title: {
      display: true,
      text: 'Chart.js Line Chart',
    },
  },
  scales: {
    y: {
      type: 'linear',
      min: 3550,
      max: 3620,
    },
    x: {
      type: 'realtime' as const,
      realtime: {
        duration: 3000,
      },
      // Change options only for THIS AXIS
    },
  },
}
ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  BarElement,
  Title,
  Tooltip,
  Legend,
  StreamingPlugin,
)

export type Datapoint = {
  x: number
  y: number
}
type EegChartProps = {
  eegData: Datapoint[]
  triggerData: Datapoint
}

export const EegChartSteaming = ({ eegData, triggerData }: EegChartProps) => {
  const chartRef = useRef<any>(null)

  const [chartData, setChartData] = useState<ChartData<'line' | 'bar'>>({
    datasets: [
      {
        label: 'Eeg data',
        data: eegData,
        borderColor: 'rgb(255, 99, 132)',
        backgroundColor: 'rgba(255, 99, 132, 0.5)',
        pointRadius: 0,
      },
      {
        type: 'bar',
        label: 'Triggers',
        data: [triggerData],
        borderColor: 'rgb(255, 99, 132)',
        backgroundColor: 'rgba(255, 99, 132, 0.5)',
      },
    ],
  })

  useEffect(() => {
    const eegOldData = chartData.datasets[0].data
    const triggerOldData = chartData.datasets[1].data

    const newChartData = {
      datasets: [
        {
          type: 'line' as const,
          label: 'Eeg data',
          data: eegOldData.concat(eegData),
          borderColor: 'rgb(255, 99, 132)',
          backgroundColor: 'rgba(255, 99, 132, 0.5)',
          pointRadius: 0,
        },
        {
          type: 'bar' as const,
          label: 'Triggers',
          data: triggerOldData, //triggerOldData.concat(triggerData),
          borderColor: 'rgb(255, 99, 132)',
          backgroundColor: 'rgba(255, 99, 132, 0.5)',
        },
      ],
    }

    setChartData(newChartData)
    chartRef.current?.update('quiet')
  }, [eegData])

  useEffect(() => {
    const eegOldData = chartData.datasets[0].data
    const triggerOldData = chartData.datasets[1].data
    console.log('in here')
    const newChartData = {
      datasets: [
        {
          type: 'line' as const,
          label: 'Eeg data',
          data: eegOldData,
          borderColor: 'rgb(255, 99, 132)',
          backgroundColor: 'rgba(255, 99, 132, 0.5)',
          pointRadius: 0,
        },
        {
          type: 'bar' as const,
          label: 'Triggers',
          data: triggerOldData.concat(triggerData),
          borderColor: 'rgb(255, 99, 132)',
          backgroundColor: 'rgba(255, 99, 132, 0.5)',
        },
      ],
    }

    //setChartData(newChartData)
    chartRef.current?.update('quiet')
  }, [triggerData])

  return <Chart type={'line'} id={'eeg-chart'} ref={chartRef} data={chartData} options={options} />
}
