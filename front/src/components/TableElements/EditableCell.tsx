import React, { useEffect, useState } from 'react'
import { useFocusMemo } from '../../utils'
import styled from 'styled-components'

const EditableCell = ({
  value: initialValue,
  row: { index },
  column: { id },
  updateData, // This is a custom function that we supplied to our table instance
}: any) => {
  // We need to keep and update the state of the cell normally
  const [value, setValue] = useState(initialValue)
  const [changed, setChanged] = useState<boolean>(false)
  const [toggle, setToggle] = useState<boolean>(true)

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
      updateData(index, id, value, false)
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
    <span onDoubleClick={onDoubleClick}>{value}</span>
  ) : (
    <CellInput value={value} onChange={onChange} onBlur={onBlur} ref={inputRef} onKeyPress={handleKeyPress} />
  )
}

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
