import React, { useEffect, useState } from 'react'
import styled from 'styled-components'

import { GridComponent } from 'components/GridComponent'
import { AngleSelector } from 'components/AngleSelector'
import { IntensitySelector } from 'components/IntensitySelector'

const Wrapper = styled.div`
  display: grid;
  grid-template-columns: 1fr 1fr;
  grid-template-rows: auto 1fr;
  gap: 1rem;
  padding: 1rem;
`

const styledPanel = `
  padding: 25px 0px 40px 35px;
  border-radius: 5px;
  background-color: #f7f7f7;
  box-shadow: 0px 3px 6px rgba(0, 0, 0, 0.1);/
`

const GridPanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 1 / 2;
  width: 600px;
  height: 500px;
  ${styledPanel}
`

const TabBar = styled.div`
  margin: 0.5rem;

  a {
    text-decoration: none;
    color: #505050;
    padding: 0.5rem;
    display: inline-block;
    transition: color 0.3s ease;

    &:hover {
      color: #303030;
    }

    &.active {
      color: #222222;
      font-weight: bold;
    }
  }
`

const AnglePanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 2 / 3;
  display: flex;
  flex-direction: column;
  gap: 1.5rem;
  width: 600px;
  height: 500px;
  ${styledPanel}
`

const IntensityPanel = styled.div`
  grid-row: 1 / 2;
  grid-column: 3 / 3;
  display: flex;
  flex-direction: column;
  gap: 1.5rem;
  width: 100px;
  height: 500px;
  ${styledPanel}
`

export const ExperimentControl = () => {
  const [activeTab, setActiveTab] = useState<'singleLocation' | 'multipleLocations'>('singleLocation')
  const [selectedAngles, setSelectedAngles] = useState<number[]>([])

  const handleIntensityChange = (value: number) => {
    console.log(`Selected intensity: ${value} V/m`)
  }

  useEffect(() => {
    if (activeTab === 'singleLocation') {
      setSelectedAngles([])
    }
  }, [activeTab])

  return (
    <>
      <TabBar>
        <a
          href="#"
          onClick={() => setActiveTab('singleLocation')}
          className={activeTab === 'singleLocation' ? 'active' : ''}
        >
          Single Location
        </a>
        <a
          href="#"
          onClick={() => setActiveTab('multipleLocations')}
          className={activeTab === 'multipleLocations' ? 'active' : ''}
        >
          Multiple Locations
        </a>
      </TabBar>

      <Wrapper>
        <GridPanel>
          {activeTab === 'singleLocation' && <GridComponent />}
          {activeTab === 'multipleLocations' && <GridComponent multiSelectMode={true} />}
        </GridPanel>
        <AnglePanel>
        {activeTab === 'singleLocation' &&
          <AngleSelector selectedAngles={selectedAngles} setSelectedAngles={setSelectedAngles} />
        }
        {activeTab === 'multipleLocations' &&
          <AngleSelector selectedAngles={selectedAngles} setSelectedAngles={setSelectedAngles} multiSelectMode={true} />
        }
        </AnglePanel>
        <IntensityPanel>
        {activeTab === 'singleLocation' &&
          <IntensitySelector min={0} max={150} threshold={100} onValueChange={handleIntensityChange}/>
        }
        {activeTab === 'multipleLocations' &&
          <IntensitySelector min={0} max={150} threshold={100} onValueChange={handleIntensityChange}/>
        }
        </IntensityPanel>
      </Wrapper>
    </>
  )
}
