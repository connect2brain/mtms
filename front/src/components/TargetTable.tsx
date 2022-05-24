import React, { useState } from 'react'

import { useTable, usePagination, Row } from 'react-table'
import styled from 'styled-components'
import { ChangeableKey } from '../types/ros'

const NotEditableCell = ({ value: initialValue }: any) => {
  return <DisabledInput value={initialValue} disabled={true} />
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

  const inputRef: React.RefObject<HTMLInputElement> = React.createRef()

  const onChange = (e: any) => {
    setValue(e.target.value)
    setChanged(true)
  }

  // We'll only update the external data when the input is blurred
  const onBlur = () => {
    if (changed) {
      updateData(index, id, value)
      setChanged(false)
    }
  }

  const handleKeyPress = (event: any) => {
    if (event.key === 'Enter') {
      inputRef.current?.blur()
    }
  }

  // If the initialValue is changed external, sync it up with our state
  React.useEffect(() => {
    setValue(value)
  }, [value])

  return <input value={value} onChange={onChange} onBlur={onBlur} ref={inputRef} onKeyPress={handleKeyPress} />
}

const defaultColumn = {
  Cell: NotEditableCell,
}

type TableProps = {
  columns: any[]
  data: any[]
  updateData: (rowIndex: number, key: ChangeableKey, value: any) => void
  skipPageReset: boolean
}

export const TargetTable = ({ columns, data, updateData, skipPageReset }: TableProps) => {
  const {
    getTableProps,
    getTableBodyProps,
    headerGroups,
    prepareRow,
    page,
    canPreviousPage,
    canNextPage,
    pageOptions,
    pageCount,
    gotoPage,
    nextPage,
    previousPage,
    setPageSize,
    state: { pageIndex, pageSize },
  } = useTable(
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
        <thead>
          {headerGroups.map((headerGroup) => (
            <TableRow {...headerGroup.getHeaderGroupProps()} key={headerGroup.getHeaderGroupProps().key}>
              {headerGroup.headers.map((column) => (
                <Th {...column.getHeaderProps()} key={column.getHeaderProps().key}>
                  {column.render('Header')}
                </Th>
              ))}
            </TableRow>
          ))}
        </thead>

        <tbody {...getTableBodyProps()}>
          {page.map((row: Row) => {
            prepareRow(row)
            return (
              <TableRow {...row.getRowProps()} key={row.getRowProps().key}>
                {row.cells.map((cell) => {
                  return (
                    <Td {...cell.getCellProps()} key={cell.getCellProps().key}>
                      {cell.render('Cell')}
                    </Td>
                  )
                })}
              </TableRow>
            )
          })}
        </tbody>
      </TargetsTable>
      <Pagination>
        <button onClick={() => gotoPage(0)} disabled={!canPreviousPage}>
          {'<<'}
        </button>{' '}
        <button onClick={() => previousPage()} disabled={!canPreviousPage}>
          {'<'}
        </button>{' '}
        <button onClick={() => nextPage()} disabled={!canNextPage}>
          {'>'}
        </button>{' '}
        <button onClick={() => gotoPage(pageCount - 1)} disabled={!canNextPage}>
          {'>>'}
        </button>{' '}
        <span>
          Page{' '}
          <strong>
            {pageIndex + 1} of {pageOptions.length}
          </strong>{' '}
        </span>
        <span>
          | Go to page:{' '}
          <input
            type='number'
            defaultValue={pageIndex + 1}
            onChange={(e) => {
              const page = e.target.value ? Number(e.target.value) - 1 : 0
              gotoPage(page)
            }}
            style={{ width: '100px' }}
          />
        </span>{' '}
        <select
          value={pageSize}
          onChange={(e) => {
            setPageSize(Number(e.target.value))
          }}
        >
          {[10, 20, 30, 40, 50].map((pageSize) => (
            <option key={pageSize} value={pageSize}>
              Show {pageSize}
            </option>
          ))}
        </select>
      </Pagination>
    </TargetsContainer>
  )
}

const Th = styled.th`
  padding: 0.5rem 1rem;
  text-align: left;
`
const Td = styled.td`
  padding: 0.5rem 1rem;
  border: #e0e0e0;

  textarea,
  input {
    border: 0;
    background-color: inherit;
    font-size: 1rem;
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Oxygen', 'Ubuntu', 'Cantarell', 'Fira Sans',
      'Droid Sans', 'Helvetica Neue', sans-serif;
  }
`
const DisabledInput = styled.input`
  color: ${(p) => p.theme.colors.primary};
  border: 0;
  background-color: inherit;
  font-size: 1rem;
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Oxygen', 'Ubuntu', 'Cantarell', 'Fira Sans',
    'Droid Sans', 'Helvetica Neue', sans-serif;
`

const TableRow = styled.tr`
  border-bottom: 1px solid #dddddd;

  :nth-of-type(even) {
    background-color: #f3f3f3;
  }

  :last-of-type {
    border-bottom: 2px solid #797979;
  }
`

const TargetsContainer = styled.div`
  overflow-x: auto;
`

const TargetsTable = styled.table`
  border-collapse: collapse;
  box-shadow: 0 0 20px rgba(0, 0, 0, 0.15);
`

const Pagination = styled.div``
