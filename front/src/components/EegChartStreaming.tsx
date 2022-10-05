import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  ChartOptions,
  ChartData,
} from 'chart.js'
import 'chartjs-adapter-date-fns'
import StreamingPlugin from 'chartjs-plugin-streaming'

import { Line } from 'react-chartjs-2'
import React, { useEffect, useRef, useState } from 'react'

const options: ChartOptions<'line'> = {
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
      min: 3020,
      max: 3220,
    },
    x: {
      type: 'realtime' as const,

      // Change options only for THIS AXIS
    },
  },
}
ChartJS.register(CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend, StreamingPlugin)

type Datapoint = {
  x: number
  y: number
}
type EegChartProps = {
  data: Datapoint[]
}

export const EegChartSteaming = ({ data }: EegChartProps) => {
  const chartRef = useRef<any>(null)

  const [chartData, setChartData] = useState<ChartData<'line'>>({
    datasets: [
      {
        label: 'Eeg data',
        data: [],
        borderColor: 'rgb(255, 99, 132)',
        backgroundColor: 'rgba(255, 99, 132, 0.5)',
        pointRadius: 0,
      },
    ],
  })

  useEffect(() => {
    const oldData = chartData.datasets[0].data

    const mappedData = data.map((point) => {
      const timeInSeconds = point.x / 1000000
      return {
        y: point.y,
        x: Math.round(timeInSeconds), //new Date(point.x),
      }
    })

    const newChartData = {
      datasets: [
        {
          label: 'Eeg data',
          data: oldData.concat(mappedData),
          borderColor: 'rgb(255, 99, 132)',
          backgroundColor: 'rgba(255, 99, 132, 0.5)',
          pointRadius: 0,
        },
      ],
    }
    setChartData(newChartData)
    chartRef.current?.update('quiet')
  }, [data])

  return <Line id={'eeg-chart'} ref={chartRef} data={chartData} options={options} />
}
