import React, { useEffect, useState } from 'react'
import { getSequenceIndexFromRowId, isOfChangeableKey, useFocusMemo } from 'utils'
import styled from 'styled-components'
import Rectangle from '../Rectangle'
import { CellProps } from 'types/table'
import useStore from 'providers/state'
import { updateTarget } from 'services/ros'
import { ChangeableKey } from 'types/target'

interface EditableCellProps extends CellProps {
  expandElement?: any
  whiteSpace?: boolean
  updateData: (rowIndex: number, columnName: string, value: any, toggle: boolean) => void //TODO: this is not needed
}

export const EditableSequenceTableCell = (props: EditableCellProps) => {
  const { row } = props
  const isTarget = props.row.depth > 0
  const { targets, setTargets, sequences, setSequences } = useStore()

  const sequenceIndex = getSequenceIndexFromRowId(row.id)

  const updateTargetData = (rowIndex: number, columnName: string, value: any, toggle: boolean) => {
    console.log('updating target data')
    const targetIndex = sequences[sequenceIndex].pulses[rowIndex].targetIndex
    const target = targets[targetIndex]
    const key: string = columnName.slice(3).toLowerCase()
    updateTarget(target, key, value, toggle, targets, setTargets)
  }

  const updateSequenceData = (rowIndex: number, columnName: string, value: any, toggle: boolean) => {
    const key: string = columnName.slice(3).toLowerCase()
    const sequence = sequences[rowIndex]
    const newSequence = {
      ...sequence,
      [key]: value,
    }
    const newSequences = sequences.filter((seq) => seq.name !== sequence.name)
    newSequences.splice(rowIndex, 0, newSequence)
    setSequences(newSequences)
  }

  return <EditableCell {...props} updateData={isTarget ? updateTargetData : updateSequenceData} />
}

export const EditableTargetTableCell = (props: EditableCellProps) => {
  const { row } = props
  const { targets, setTargets } = useStore()

  const updateTargetData = (rowIndex: number, columnName: string, value: any, toggle: boolean) => {
    console.log('updating target data')
    const target = targets[rowIndex]
    updateTarget(target, columnName, value, toggle, targets, setTargets)
  }

  return <EditableCell {...props} updateData={updateTargetData} />
}

const EditableCell = ({
  value: initialValue,
  row,
  column,
  updateData,
  expandElement,
  whiteSpace,
}: EditableCellProps) => {
  // We need to keep and update the state of the cell normally
  const [value, setValue] = useState(initialValue)
  const [changed, setChanged] = useState<boolean>(false)
  const [beforeChange, setBeforeChange] = useState(initialValue)
  const [toggle, setToggle] = useState<boolean>(true)

  const { index, depth } = row

  const [inputRef, setInputFocus] = useFocusMemo()

  const onChange = (e: any) => {
    setValue(e.target.value)
    setChanged(true)
  }

  // We'll only update the external data when the input is blurred
  const onBlur = () => {
    if (value && value.length === 0) {
      setValue(beforeChange)
      // do not update data if value is not changed
      return
    } else {
      setBeforeChange(value)
    }

    if (changed && column.id) {
      updateData(index, column.id, value, false)
      setChanged(false)
    }
    setToggle(true)
  }

  const handleKeyPress = (event: any) => {
    if (event.key === 'Enter') {
      inputRef.current?.blur()
    }
  }

  // If the initialValue is changed external, sync it up with our state
  useEffect(() => {
    setValue(value)
  }, [value])
  const onDoubleClick = () => {
    setToggle(false)

    // Needs to be in setTimeout so the input element can render, otherwise it would be impossible to setInputFocus
    // as the element does not exist
    setTimeout(() => {
      setInputFocus()
    }, 100)
  }
  //console.log(row)
  return (
    <Margin>
      {whiteSpace ? (
        <>
          <WhiteSpace cols={depth} />
          <Rectangle color={'red'} />
        </>
      ) : null}
      {toggle ? (
        <Container onDoubleClick={onDoubleClick}>
          <span>{value}</span>
          {expandElement}
        </Container>
      ) : (
        <CellInput value={value} onChange={onChange} onBlur={onBlur} ref={inputRef} onKeyPress={handleKeyPress} />
      )}
    </Margin>
  )
}

const Margin = styled.span`
  display: flex;
`

const WhiteSpace = styled.span<{
  cols: number
}>`
  background-color: rgba(255, 255, 255, 0);
  width: ${(p) => p.cols * 20}px;
  height: 100%;
  overflow: hidden;
  writing-mode: horizontal-tb;
  border: 0;
`
const Container = styled.span`
  display: flex;
  width: 100%;
  border: 0;
  background-color: inherit;
  font-size: 1rem;
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Oxygen', 'Ubuntu', 'Cantarell', 'Fira Sans',
    'Droid Sans', 'Helvetica Neue', sans-serif;
  overflow: auto;
  overflow-y: hidden;
  min-height: 1rem;
`

const CellInput = styled.input`
  all: unset;
  width: 100%;
  border: 0;
  background-color: inherit;
  font-size: 1rem;
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Oxygen', 'Ubuntu', 'Cantarell', 'Fira Sans',
    'Droid Sans', 'Helvetica Neue', sans-serif;
`
export default EditableCell
