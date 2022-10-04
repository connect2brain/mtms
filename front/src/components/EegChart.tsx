import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
} from 'chart.js'
import { Line } from 'react-chartjs-2'
import React, { useEffect } from 'react'

const options = {
  responsive: true,
  animation: {
    duration: 0, // general animation time
  },
  scales: {
    y: {
      min: 3070,
      max: 3150,
    },
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
}
ChartJS.register(CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend)
type EegChartProps = {
  data: number[]
}

export const EegChart = ({ data }: EegChartProps) => {
  const chartData = {
    labels: Array.from(data.keys()),
    datasets: [
      {
        label: 'Eeg data',
        data: data,
        borderColor: 'rgb(255, 99, 132)',
        backgroundColor: 'rgba(255, 99, 132, 0.5)',
        pointRadius: 0,
      },
    ],
  }
  return <Line id={'eeg-chart'} data={chartData} options={options} />
}
