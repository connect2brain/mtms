import React from 'react'

import { useTable, usePagination, Row, Cell } from 'react-table';
import styled from 'styled-components';

type EditableCellProps = {
  value: any
  row: { index: number }
  column: { id: number }
  updateData: (index: number, id: number, value: any) => void
}

type CellProps = {
  value: any
  column: number
}

type RowProps = {
  original: any
}

type EditableProps = {
  cell: CellProps
  row: RowProps
  updateData: (index: number, id: number, value: any) => void
}
const EditableCell = ({
  value: initialValue,
  row: { index },
  column: { id },
  updateData, // This is a custom function that we supplied to our table instance
}: any) => {
  // We need to keep and update the state of the cell normally
  const [value, setValue] = React.useState(initialValue)

  const onChange = (e: any) => {
    setValue(e.target.value)
  }

  // We'll only update the external data when the input is blurred
  const onBlur = () => {
    console.log('initialValue', initialValue)
    console.log('index', index)
    console.log('id', id)
    console.log('new value', value)
    updateData(index, id, value)
  }

  // If the initialValue is changed external, sync it up with our state
  React.useEffect(() => {
    setValue(value)
  }, [value])

  return <input value={value} onChange={onChange} onBlur={onBlur} />
}

const defaultColumn = {
  Cell: EditableCell,
}

type TableProps = {
  columns: any[]
  data: any[]
  updateData: (rowIndex: number, columnId: number, value: any) => void
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
    <>
      <table {...getTableProps()}>
        <thead>
          {headerGroups.map((headerGroup) => (
            <tr {...headerGroup.getHeaderGroupProps()} key={headerGroup.getHeaderGroupProps().key}>
              {headerGroup.headers.map((column) => (
                <th {...column.getHeaderProps()} key={column.getHeaderProps().key}>
                  {column.render('Header')}
                </th>
              ))}
            </tr>
          ))}
        </thead>

        <tbody {...getTableBodyProps()}>
        {page.map((row: Row, i: number) => {
          prepareRow(row)
          return (
            <tr {...row.getRowProps()} key={row.getRowProps().key}>
              {row.cells.map(cell => {
                return <td {...cell.getCellProps()} key={cell.getCellProps().key}>{cell.render('Cell')}</td>
              })}
            </tr>
          )
        })}
        </tbody>
      </table>
      <div className="pagination">
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
            type="number"
            defaultValue={pageIndex + 1}
            onChange={e => {
              const page = e.target.value ? Number(e.target.value) - 1 : 0
              gotoPage(page)
            }}
            style={{ width: '100px' }}
          />
        </span>{' '}
        <select
          value={pageSize}
          onChange={e => {
            setPageSize(Number(e.target.value))
          }}
        >
          {[10, 20, 30, 40, 50].map(pageSize => (
            <option key={pageSize} value={pageSize}>
              Show {pageSize}
            </option>
          ))}
        </select>
      </div>
    </>
  )
}

const Pagination = styled.div`

`