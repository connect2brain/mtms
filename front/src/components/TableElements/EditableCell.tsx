import React, { useEffect, useState } from 'react'
import { useFocusMemo } from '../../utils'
import styled from 'styled-components'
import Expand from '../Expand'

interface EditableCellProps {
  value: string
  row: any
  column: any
  updateData: (rowIndex: number, columnName: string, value: any, toggle: boolean) => void
  expandElement?: any
}

const EditableCell = ({ value: initialValue, row, column, updateData, expandElement }: EditableCellProps) => {
  // We need to keep and update the state of the cell normally
  const [value, setValue] = useState(initialValue)
  const [changed, setChanged] = useState<boolean>(false)
  const [toggle, setToggle] = useState<boolean>(true)

  const { index, canExpand } = row

  //const inputRef: React.RefObject<HTMLInputElement> = React.createRef()
  const [inputRef, setInputFocus] = useFocusMemo()

  const onChange = (e: any) => {
    if (e.target.value.length > 0) {
      setValue(e.target.value)
      setChanged(true)
    }
  }

  // We'll only update the external data when the input is blurred
  const onBlur = () => {
    if (changed) {
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
    setTimeout(() => {
      console.log(toggle)
      setInputFocus()
    }, 100)
  }

  return toggle ? (
    <ExpandContainer>
      <span onDoubleClick={onDoubleClick}>{value}</span> {expandElement}
    </ExpandContainer>
  ) : (
    <CellInput value={value} onChange={onChange} onBlur={onBlur} ref={inputRef} onKeyPress={handleKeyPress} />
  )
}

const ExpandContainer = styled.span`
  display: flex !important;
  justify-content: space-between !important;
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
