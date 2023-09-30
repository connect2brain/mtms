import React, { useState } from 'react'
import styled from 'styled-components'
import { GridControls } from './GridControls'

interface Point {
  x: number
  y: number
}

interface GridComponentProps {
  multiSelectMode?: boolean
}

interface GridCellProps {
  isSelected: boolean
  isOriginLine?: boolean
}

const GridInternalContainer = styled.div`
  display: flex;
  gap: 20px;
`

const GridContainer = styled.div`
  position: relative;
  width: max-content;
`

const Grid = styled.div`
  display: flex;
  flex-direction: column;
`

const GridRow = styled.div`
  display: flex;
`

const GridCell = styled.div<GridCellProps>`
  width: 10px;
  height: 10px;
  border: 1px solid #ccc;

  background-color: ${props =>
    props.isSelected ? '#007bff' :
    props.isOriginLine ? '#ddd' : 'transparent'};

  &:hover {
    background-color: rgba(0, 123, 255, 0.4);
  }

  cursor: pointer;
  transition: background-color 0.2s;
`

const AxisLabel = styled.div`
  position: absolute;
`

const XAxisCenterLabel = styled(AxisLabel)`
  bottom: -30px;
  left: 50%;
  transform: translateX(-50%);
`

const LeftLabel = styled(AxisLabel)`
  bottom: -30px;
  left: 0;
`

const RightLabel = styled(AxisLabel)`
  bottom: -30px;
  right: 0;
`

const YAxisCenterLabel = styled(AxisLabel)`
  top: 60%;
  left: -30px;
  transform: translateY(-50%) rotate(-90deg);
  transform-origin: top left;
`

const PosteriorLabel = styled(AxisLabel)`
  top: 70px;
  left: -30px;
  transform: rotate(-90deg);
  transform-origin: top left;
`

const AnteriorLabel = styled(AxisLabel)`
  bottom: 5px;
  left: -11px;
  transform: rotate(-90deg);
  transform-origin: bottom left;
`

const CoordinateDisplay = styled.div<{ isActive: boolean }>`
  position: absolute;
  top: -18px;
  right: -10px;
  background-color: #f8f8f8;
  font-size: 0.8em;
  color: ${props => props.isActive ? 'black' : 'rgba(0, 0, 0, 0.5)'};
  transition: color 0.3s ease;
  display: flex;
  align-items: center;
`

const CoordinateValue = styled.span`
  width: 24px;
  text-align: right;
  margin-right: 12px;
`

export const GridComponent: React.FC<GridComponentProps> = ({ multiSelectMode = false }) => {
    const [shape, setShape] = useState<'circle' | 'square' | null>(null)
    const [shapeSize, setShapeSize] = useState<number>(0)
    const [selectedPoints, setSelectedPoints] = useState<Point[]>([])
    const [isMouseDown, setIsMouseDown] = useState<boolean>(false)
    const [dragAction, setDragAction] = useState<'selecting' | 'deselecting'>('selecting')
    const [hoveredPoint, setHoveredPoint] = useState<Point | null>(null)
    const [isHoveringOverGrid, setIsHoveringOverGrid] = useState<boolean>(false)

    const resetGrid = () => {
        setSelectedPoints([])
    }

    const isPointSelected = (x: number, y: number): boolean =>
      selectedPoints.some(point => point.x === x && point.y === y)

    const selectCell = (x: number, y: number) => {
      if (!isPointSelected(x, y)) {
        setSelectedPoints(prevPoints => [...prevPoints, { x, y }])
      }
    }

    const deselectCell = (x: number, y: number) => {
      setSelectedPoints(prevPoints => prevPoints.filter(point => !(point.x === x && point.y === y)))
    }

    const handleCellMouseDown = (x: number, y: number) => {
      setIsMouseDown(true)
      if (!multiSelectMode) {
          setSelectedPoints([])
      }
      if (isPointSelected(x, y)) {
          setDragAction('deselecting')
          deselectCell(x, y)
      } else {
          setDragAction('selecting')
          selectCell(x, y)
      }
    }

    const handleCellMouseEnter = (x: number, y: number) => {
      if (isMouseDown && multiSelectMode) {
        if (dragAction === 'selecting') {
          selectCell(x, y)
        } else {
          deselectCell(x, y)
        }
      }
    }

    const handleMouseUp = () => {
      setIsMouseDown(false)
    }

    const handleShapeSelected = (selectedShape: 'circle' | 'square', size: number) => {
        const newSelectedPoints: Point[] = []

        if (selectedShape === 'circle') {
            for (let x = -15; x <= 15; x++) {
                for (let y = -15; y <= 15; y++) {
                    if (x * x + y * y <= size * size) {
                        newSelectedPoints.push({ x, y })
                    }
                }
            }
        } else if (selectedShape === 'square') {
            for (let x = -size; x <= size; x++) {
                for (let y = -size; y <= size; y++) {
                    newSelectedPoints.push({ x, y })
                }
            }
        }

        setSelectedPoints(newSelectedPoints)
    }

    const handleDecimate = (n: number) => {
      setSelectedPoints((prevPoints) =>
        prevPoints.filter(point =>
          (point.x % n === 0) && (point.y % n === 0)
        )
      )
    }

    return (
        <div>
          <GridInternalContainer>
            <GridContainer
                onMouseUp={handleMouseUp}
                onMouseEnter={() => setIsHoveringOverGrid(true)}
                onMouseLeave={() => {
                  setIsHoveringOverGrid(false)
                  setHoveredPoint(null)
                }}
            >
            <Grid>
            {Array.from({ length: 31 }).map((_, rowIndex) => (
                <GridRow key={rowIndex}>
                {Array.from({ length: 31 }).map((_, colIndex) => (
                    <GridCell
                      key={colIndex}
                      isSelected={isPointSelected(colIndex - 15, rowIndex - 15)}
                      isOriginLine={colIndex === 15 || rowIndex === 15}
                      onMouseDown={() => handleCellMouseDown(colIndex - 15, rowIndex - 15)}
                      onMouseEnter={() => {
                        handleCellMouseEnter(colIndex - 15, rowIndex - 15)
                        setHoveredPoint({ x: colIndex - 15, y: rowIndex - 15 })
                      }}
                      onMouseLeave={() => {
                        setHoveredPoint(null)
                      }}
                    />
                ))}
                </GridRow>
            ))}
            </Grid>
            <XAxisCenterLabel>x (mm)</XAxisCenterLabel>

        <LeftLabel>Left</LeftLabel>
        <RightLabel>Right</RightLabel>
        <YAxisCenterLabel>y (mm)</YAxisCenterLabel>
        <PosteriorLabel>Posterior</PosteriorLabel>
        <AnteriorLabel>Anterior</AnteriorLabel>
        <CoordinateDisplay isActive={isHoveringOverGrid}>
          x: <CoordinateValue>{hoveredPoint ? hoveredPoint.x : '–'}</CoordinateValue>
          y: <CoordinateValue>{hoveredPoint ? hoveredPoint.y : '–'}</CoordinateValue>
        </CoordinateDisplay>
        </GridContainer>
        { multiSelectMode &&
          <GridControls
            onShapeSelected={handleShapeSelected}
            onReset={resetGrid}
            onDecimate={handleDecimate}
          />
        }
        </GridInternalContainer>
    </div>
  )
}
