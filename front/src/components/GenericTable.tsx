import React, { useState } from 'react'

import { useTable, Row, useExpanded } from 'react-table'
import styled from 'styled-components'
import { ChangeableKey } from 'types/target'
import useStore from 'providers/state'
import { ControlledMenu, MenuItem, useMenuState } from '@szhsin/react-menu'
import '@szhsin/react-menu/dist/index.css'
import { Pulse, PulseSequence } from '../types/pulseSequence'
import NotEditableCell from './TableElements/NotEditableCell'
import SelectableTableRow from './TableElements/SelectableTableRow'

const defaultColumn = {
  Cell: NotEditableCell,
}

interface TableProps {
  columns: any[]
  data: any[]
  updateData: (rowIndex: number, key: ChangeableKey, value: any, toggle: boolean) => void
  createMenu: any
}

export const GenericTable = ({ columns, data, updateData, createMenu }: TableProps) => {
  const {
    getTableProps,
    getTableBodyProps,
    headerGroups,
    prepareRow,
    rows,
    state: { expanded },
  } = useTable(
    {
      columns,
      data,
      defaultColumn,
      autoResetExpanded: false,
      /* updateMyData isn't part of the API, but anything we put into these options will automatically
            be available on the instance. That way we can call this function from our cell renderer and updateData */
      updateData,
    },
    useExpanded,
  )

  const [menuProps, toggleMenu] = useMenuState()
  const [anchorPoint, setAnchorPoint] = useState({ x: 0, y: 0 })

  const onContextMenu = (event: any) => {
    event.preventDefault()
    setAnchorPoint({ x: event.clientX, y: event.clientY })
    toggleMenu(true)
  }

  return (
    <TargetsContainer onContextMenu={onContextMenu}>
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
                    <Td {...cell.getCellProps()} key={cell.getCellProps().key}>
                      {cell.render('Cell')}
                    </Td>
                  )
                })}
              </SelectableTableRow>
            )
          })}
        </Tbody>
      </TargetsTable>

      <pre>
        <code>{JSON.stringify({ expanded: expanded }, null, 2)}</code>
      </pre>

      <ControlledMenu {...menuProps} anchorPoint={anchorPoint} onClose={() => toggleMenu(false)}>
        {createMenu()}
      </ControlledMenu>
    </TargetsContainer>
  )
}

const Th = styled.th`
  padding: 0.25rem 0.5rem;
  text-align: left;
  border-top: none !important;
  border-bottom: none !important;

  :last-of-type {
    box-shadow: inset 0 1px 0 ${(p) => p.theme.colors.gray}, inset 0 -1px 0 ${(p) => p.theme.colors.gray};
  }

  :not(:last-of-type) {
    box-shadow: inset 0 1px 0 ${(p) => p.theme.colors.gray}, inset 0 -1px 0 ${(p) => p.theme.colors.gray},
      inset -1px 0 0 ${(p) => p.theme.colors.gray};
  }
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

const HeaderTableRow = styled.tr`
  :nth-of-type(even) {
    background-color: #f3f3f3;
  }

  :nth-of-type(odd) {
    background-color: #ffffff;
  }
`

const TargetsContainer = styled.div`
  overflow-y: auto;
  overflow-x: hidden;
  width: fit-content;
  max-height: 600px;
  max-width: 60%;
`

const TargetsTable = styled.table`
  border-collapse: collapse;
  box-shadow: 0 0 20px rgba(0, 0, 0, 0.15);
  table-layout: fixed;
  width: 100%;
`
