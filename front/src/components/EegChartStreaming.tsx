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
import theme from '../styles/theme'

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
        duration: 5000,
      },
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

export type DatapointWithEventType = {
  x: number
  y: number
  eventType: number
}

export type EegChartProps = {
  eegData: Datapoint[]
  latestEvent: DatapointWithEventType
}

const datasetDefaults = [
  {
    borderColor: theme.colors.red,
    backgroundColor: theme.colors.red,
    pointRadius: 0,
    label: 'Eeg data',
  },
  {
    type: 'bar' as const,
    label: 'Pulse',
    borderColor: theme.colors.blue,
    backgroundColor: theme.colors.blue,
    barThickness: 1,
  },
  {
    type: 'bar' as const,
    label: 'Charge',
    borderColor: theme.colors.yellow,
    backgroundColor: theme.colors.yellow,
    barThickness: 1,
  },
  {
    type: 'bar' as const,
    label: 'Discharge',
    borderColor: theme.colors.brown,
    backgroundColor: theme.colors.brown,
    barThickness: 1,
  },
  {
    type: 'bar' as const,
    label: 'Signal out',
    borderColor: theme.colors.green,
    backgroundColor: theme.colors.green,
    barThickness: 1,
  },
]

export const EegChartSteaming = ({ eegData, latestEvent }: EegChartProps) => {
  const chartRef = useRef<any>(null)

  const [chartData, setChartData] = useState<ChartData<'line' | 'bar'>>({
    datasets: [
      {
        data: eegData,
        ...datasetDefaults[0],
      },
      ...datasetDefaults.slice(1).map((dataset) => {
        return {
          ...dataset,
          data: [latestEvent],
        }
      }),
    ],
  })

  useEffect(() => {
    const eegOldData = chartData.datasets[0].data

    const newChartData = {
      datasets: [
        {
          data: eegOldData.concat(eegData),
          ...datasetDefaults[0],
        },
        ...datasetDefaults.slice(1).map((dataset, index) => {

          return {
            ...dataset,
            data:
              latestEvent.eventType === index ? chartData.datasets[index + 1].data : chartData.datasets[index + 1].data,
          }
        }),
      ],
    }

    setChartData(newChartData)
    chartRef.current?.update('quiet')
  }, [eegData])

  useEffect(() => {
    const eegOldData = chartData.datasets[0].data

    const newChartData = {
      datasets: [
        {
          data: eegOldData,
          ...datasetDefaults[0],
        },
        ...datasetDefaults.slice(1).map((dataset, index) => {
          return {
            ...dataset,
            data:
              latestEvent.eventType === index
                ? chartData.datasets[index + 1].data.concat(latestEvent)
                : chartData.datasets[index + 1].data,
          }
        }),
      ],
    }

    setChartData(newChartData)
    chartRef.current?.update('quiet')
  }, [latestEvent])

  return <Chart type={'line'} id={'eeg-chart'} ref={chartRef} data={chartData} options={options} />
}
