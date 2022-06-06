import React, { useEffect, useState } from 'react'

import { useTable, usePagination, Row } from 'react-table'
import styled from 'styled-components'
import { ChangeableKey } from '../types/target'
import Eye from './Eye'
import useStore from '../providers/state'
import { useFocus, useFocusMemo } from '../utils'

const NotEditableCell = ({ value: initialValue }: any) => {
  return <DisabledInput value={initialValue} disabled={true} />
}

export const EyeCell = ({
  value: initialValue,
  row: { index },
  column: { id },
  updateData, // This is a custom function that we supplied to our table instance
}: any) => {
  const [visible, setVisible] = useState(initialValue)

  const onClick = (event: any) => {
    event.stopPropagation()
    const newVisible = !visible
    setVisible(newVisible)
    updateData(index, id, newVisible, true)
  }

  // If the initialValue is changed external, sync it up with our state
  React.useEffect(() => {
    setVisible(visible)
  }, [visible])

  return (
    <EyeButton onClick={onClick}>
      <Eye visible={visible} />
    </EyeButton>
  )
}

export const EditableCell = ({
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
    console.log('double clicked')
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

const SelectableTableRow = (props: any) => {
  const { index, updateData } = props
  const { targets, setTargets } = useStore((state) => state)

  const onClick = (event: any) => {
    event.preventDefault()
    console.log('clicked row', index)
    const selected = targets[index].selected

    updateData(index, 'selected', !selected, true)
  }

  return (
    <TableRow {...props} selected={targets[index].selected} onClick={onClick}>
      {props.children}
    </TableRow>
  )
}

const defaultColumn = {
  Cell: NotEditableCell,
}

type TableProps = {
  columns: any[]
  data: any[]
  updateData: (rowIndex: number, key: ChangeableKey, value: any, toggle: boolean) => void
  skipPageReset: boolean
}

export const TargetTable = ({ columns, data, updateData, skipPageReset }: TableProps) => {
  const { getTableProps, getTableBodyProps, headerGroups, prepareRow, rows } = useTable(
    {
      columns,
      data,
      defaultColumn,
      // use the skipPageReset option to disable page resetting temporarily
      autoResetPage: skipPageReset,
      // updateMyData isn't part of the API, but
      // anything we put into these options will
      // automatically be available on the instance.
      // That way we can call this function from our
      // cell renderer!
      updateData,
    },
    usePagination,
  )

  return (
    <TargetsContainer>
      <TargetsTable {...getTableProps()}>
        <Thead>
          {headerGroups.map((headerGroup) => (
            <HeaderTableRow {...headerGroup.getHeaderGroupProps()} key={headerGroup.getHeaderGroupProps().key}>
              {headerGroup.headers.map((column) => (
                <Th
                  {...column.getHeaderProps({
                    style: {
                      width: column.width,
                      minWidth: column.minWidth,
                    },
                  })}
                  key={column.getHeaderProps().key}
                >
                  {column.render('Header')}
                </Th>
              ))}
            </HeaderTableRow>
          ))}
        </Thead>

        <Tbody {...getTableBodyProps()}>
          {rows.map((row: Row) => {
            prepareRow(row)
            return (
              <SelectableTableRow
                {...row.getRowProps()}
                key={row.getRowProps().key}
                index={row.index}
                updateData={updateData}
              >
                {row.cells.map((cell) => {
                  return (
                    <Td
                      {...cell.getCellProps({
                        style: {},
                      })}
                      key={cell.getCellProps().key}
                    >
                      {cell.render('Cell')}
                    </Td>
                  )
                })}
              </SelectableTableRow>
            )
          })}
        </Tbody>
      </TargetsTable>
    </TargetsContainer>
  )
}

const Th = styled.th`
  padding: 0.25rem 0.5rem;
  text-align: left;
  border-top: none !important;
  border-bottom: none !important;
  box-shadow: inset 0 1px 0 #b0b0b0, inset 0 -1px 0 #b0b0b0;
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

const Td = styled.td`
  padding: 0.25rem 0.5rem;
  border: #e0e0e0;

  span {
    all: unset;
    border: 0;
    background-color: inherit;
    font-size: 1rem;
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Oxygen', 'Ubuntu', 'Cantarell', 'Fira Sans',
      'Droid Sans', 'Helvetica Neue', sans-serif;
    display: list-item;
    overflow: auto;
  }
`

const Thead = styled.thead`
  position: sticky;
  top: 0;
  margin: 0 0 0 0;
  width: 100%;
  z-index: 1;
`
const Tbody = styled.tbody``

const DisabledInput = styled.input`
  width: 100%;
  color: ${(p) => p.theme.colors.primary};
  border: 0;
  background-color: inherit;
  font-size: 1rem;
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Oxygen', 'Ubuntu', 'Cantarell', 'Fira Sans',
    'Droid Sans', 'Helvetica Neue', sans-serif;
`

const HeaderTableRow = styled.tr`
  :nth-of-type(even) {
    background-color: #f3f3f3;
  }

  :nth-of-type(odd) {
    background-color: #ffffff;
  }
`

const TableRow = styled.tr<{
  selected: boolean
}>`
  border-bottom: 2px solid #b0b0b0;
  background-color: ${(p) => (p.selected ? p.theme.colors.lightgray : p.theme.colors.white)};
`

const TargetsContainer = styled.div`
  overflow-y: auto;
  overflow-x: hidden;
  width: fit-content;
  max-height: 600px;
  max-width: 40%;
`

const TargetsTable = styled.table`
  border-collapse: collapse;
  box-shadow: 0 0 20px rgba(0, 0, 0, 0.15);
  table-layout: fixed;
  width: 100%;
`

const EyeButton = styled.button`
  all: unset;
`
