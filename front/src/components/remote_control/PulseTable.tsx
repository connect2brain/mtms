import React, { useState, useCallback, useRef } from 'react'
import styled from 'styled-components'

/* ── types ── */

interface Pulse {
  x: string
  y: string
  angle: string
  intensity: string
}

const emptyPulse = (): Pulse => ({ x: '', y: '', angle: '', intensity: '' })

interface PulseRow {
  id: number
  pulses: (Pulse | null)[] // length 3; null = not defined
}

/* ── validation ── */

const INT_RE = /^-?\d+$/

function isIntInRange(v: string, min: number, max: number): boolean {
  if (!INT_RE.test(v)) return false
  const n = parseInt(v, 10)
  return n >= min && n <= max
}

const validators: Record<string, (v: string) => boolean> = {
  x: (v) => isIntInRange(v, -15, 15),
  y: (v) => isIntInRange(v, -15, 15),
  angle: (v) => isIntInRange(v, 0, 359),
  intensity: (v) => isIntInRange(v, 1, 150),
}

function isPulseFullyEmpty(p: Pulse): boolean {
  return p.x === '' && p.y === '' && p.angle === '' && p.intensity === ''
}

function isPulseValid(p: Pulse): boolean {
  return (
    validators.x(p.x) &&
    validators.y(p.y) &&
    validators.angle(p.angle) &&
    validators.intensity(p.intensity)
  )
}

/* ── styled ── */

const Wrapper = styled.div`
  width: 100%;
  border-radius: 5px;
  background-color: #f7f7f7;
  box-shadow: 0 3px 6px rgba(0, 0, 0, 0.1);
  padding: 14px 16px 16px;
`

const Title = styled.h3`
  margin: 0 0 10px;
  font-size: 0.82rem;
  font-weight: 600;
  letter-spacing: 0.04em;
  text-transform: uppercase;
  color: #666;
`

const Table = styled.table`
  width: 100%;
  border-collapse: collapse;
  font-size: 0.82rem;
  line-height: 1.35;
`

const Thead = styled.thead`
  border-bottom: 1px solid #d8d8d8;
`

const Th = styled.th`
  padding: 0 0 6px;
  text-align: center;
  font-weight: 600;
  font-size: 0.68rem;
  letter-spacing: 0.04em;
  text-transform: uppercase;
  color: #888;
  white-space: nowrap;
`

const ThSpacer = styled(Th)`
  width: 6px;
  border-left: 1px solid #e0e0e0;
`

const ThPulseGroup = styled.th`
  text-align: center;
  font-weight: 700;
  font-size: 0.7rem;
  letter-spacing: 0.04em;
  text-transform: uppercase;
  color: #555;
  padding: 0 0 2px;
`

const Tr = styled.tr<{ $selected?: boolean }>`
  cursor: pointer;
  background-color: ${({ $selected }) => ($selected ? '#e3ecfa' : 'transparent')};
  &:hover {
    background-color: ${({ $selected }) => ($selected ? '#d6e3f8' : '#f0f0f0')};
  }
`

const Td = styled.td`
  padding: 3px 1px;
  text-align: center;
`

const TdSpacer = styled(Td)`
  width: 6px;
  border-left: 1px solid #e8e8e8;
`

const CellInput = styled.input<{ $invalid?: boolean }>`
  width: 42px;
  padding: 3px 4px;
  border: 1.5px solid ${({ $invalid }) => ($invalid ? '#e53935' : '#ccc')};
  border-radius: 3px;
  text-align: center;
  font-size: 0.82rem;
  font-variant-numeric: tabular-nums;
  outline: none;
  background: ${({ $invalid }) => ($invalid ? '#fff5f5' : '#fff')};
  color: ${({ $invalid }) => ($invalid ? '#c62828' : '#333')};
  &:focus {
    border-color: ${({ $invalid }) => ($invalid ? '#e53935' : '#5c8fdb')};
  }
  &:disabled {
    background: #ececec;
    border-color: #ddd;
    color: #aaa;
    cursor: not-allowed;
  }
`

const ButtonRow = styled.div`
  display: flex;
  gap: 6px;
  margin-top: 10px;
`

const SmallBtn = styled.button`
  padding: 4px 14px;
  border: none;
  border-radius: 4px;
  font-size: 0.78rem;
  font-weight: 600;
  cursor: pointer;
  color: #fff;
  background: #5c8fdb;
  &:hover {
    background: #4a7cc9;
  }
  &:disabled {
    background: #bbb;
    cursor: not-allowed;
  }
`

const Spacer = styled.div`
  flex: 1;
`

const RemoveBtn = styled(SmallBtn)`
  background: #e57373;
  &:hover {
    background: #e53935;
  }
`

const ToggleBtn = styled.button<{ $active: boolean }>`
  padding: 2px 7px;
  border: 1.5px solid ${({ $active }) => ($active ? '#5c8fdb' : '#ccc')};
  border-radius: 3px;
  font-size: 0.68rem;
  font-weight: 600;
  cursor: pointer;
  background: ${({ $active }) => ($active ? '#e3ecfa' : '#f5f5f5')};
  color: ${({ $active }) => ($active ? '#3b6fb5' : '#999')};
  &:hover {
    background: ${({ $active }) => ($active ? '#d6e3f8' : '#eee')};
  }
`

/* ── field labels ── */

const FIELDS: { key: keyof Pulse; label: string }[] = [
  { key: 'x', label: 'X' },
  { key: 'y', label: 'Y' },
  { key: 'angle', label: 'Ang' },
  { key: 'intensity', label: 'Int' },
]

/* ── component ── */

let nextId = 1

export const PulseTable: React.FC = () => {
  const [rows, setRows] = useState<PulseRow[]>([])
  const [selectedId, setSelectedId] = useState<number | null>(null)
  const fileInputRef = useRef<HTMLInputElement>(null)

  const loadFromJson = useCallback((json: string) => {
    try {
      const data = JSON.parse(json)
      if (!Array.isArray(data)) return
      const loaded: PulseRow[] = data.map((pulses: Pulse[]) => {
        const padded: (Pulse | null)[] = [...pulses.slice(0, 3)]
        while (padded.length < 3) padded.push(null)
        return { id: nextId++, pulses: padded }
      })
      setRows(loaded)
      setSelectedId(loaded.length > 0 ? loaded[loaded.length - 1].id : null)
    } catch {
      // ignore malformed files
    }
  }, [])

  const electronAPI = (window as any).electronAPI as
    | { saveFile: (name: string, content: string) => Promise<boolean>; loadFile: () => Promise<string | null> }
    | undefined

  const handleSave = useCallback(async () => {
    const data = rows.map((r) => r.pulses.filter((p): p is Pulse => p !== null))
    const json = JSON.stringify(data, null, 2)
    if (electronAPI?.saveFile) {
      await electronAPI.saveFile('pulse_table.json', json)
    } else {
      const blob = new Blob([json], { type: 'application/json' })
      const url = URL.createObjectURL(blob)
      const a = document.createElement('a')
      a.href = url
      a.download = 'pulse_table.json'
      a.click()
      URL.revokeObjectURL(url)
    }
  }, [rows, electronAPI])

  const handleLoad = useCallback(async () => {
    if (electronAPI?.loadFile) {
      const content = await electronAPI.loadFile()
      if (content !== null) loadFromJson(content)
    } else {
      fileInputRef.current?.click()
    }
  }, [electronAPI, loadFromJson])

  const handleFileInput = useCallback(
    (e: React.ChangeEvent<HTMLInputElement>) => {
      const file = e.target.files?.[0]
      if (!file) return
      const reader = new FileReader()
      reader.onload = () => loadFromJson(reader.result as string)
      reader.readAsText(file)
      e.target.value = ''
    },
    [loadFromJson],
  )

  const addRow = useCallback(() => {
    const id = nextId++
    setRows((prev) => [...prev, { id, pulses: [emptyPulse(), null, null] }])
    setSelectedId(id)
  }, [])

  const removeRow = useCallback(() => {
    if (selectedId === null) return
    setRows((prev) => {
      const idx = prev.findIndex((r) => r.id === selectedId)
      const next = prev.filter((r) => r.id !== selectedId)
      if (next.length === 0) {
        setSelectedId(null)
      } else {
        const newIdx = Math.min(idx, next.length - 1)
        setSelectedId(next[newIdx].id)
      }
      return next
    })
  }, [selectedId])

  const togglePulse = useCallback((rowId: number, pulseIdx: 1 | 2) => {
    setRows((prev) =>
      prev.map((r) => {
        if (r.id !== rowId) return r
        const pulses = [...r.pulses]
        if (pulses[pulseIdx] !== null) {
          // turning off — also turn off subsequent pulses
          for (let i = pulseIdx; i < 3; i++) pulses[i] = null
        } else {
          pulses[pulseIdx] = emptyPulse()
        }
        return { ...r, pulses }
      }),
    )
  }, [])

  const updateField = useCallback(
    (rowId: number, pulseIdx: number, field: keyof Pulse, value: string) => {
      setRows((prev) =>
        prev.map((r) => {
          if (r.id !== rowId) return r
          const pulses = [...r.pulses]
          const pulse = pulses[pulseIdx]
          if (!pulse) return r
          pulses[pulseIdx] = { ...pulse, [field]: value }
          return { ...r, pulses }
        }),
      )
    },
    [],
  )

  const isFieldInvalid = (pulse: Pulse, field: keyof Pulse): boolean => {
    const val = pulse[field]
    if (val === '') return false // empty = not yet filled, not invalid
    return !validators[field](val)
  }

  return (
    <Wrapper>
      <Title>Pulse definitions</Title>
      <Table>
        <Thead>
          <tr>
            <Th />
            {[0, 1, 2].map((pi) => (
              <React.Fragment key={pi}>
                {pi > 0 && <ThSpacer />}
                <ThPulseGroup colSpan={4}>Pulse {pi + 1}</ThPulseGroup>
              </React.Fragment>
            ))}
          </tr>
          <tr>
            <Th>#</Th>
            {[0, 1, 2].map((pi) => (
              <React.Fragment key={pi}>
                {pi > 0 && <ThSpacer />}
                {FIELDS.map((f) => (
                  <Th key={`${pi}-${f.key}`}>{f.label}</Th>
                ))}
              </React.Fragment>
            ))}
          </tr>
        </Thead>
        <tbody>
          {rows.length === 0 ? (
            <tr>
              <td
                colSpan={1 + 3 * 4 + 2}
                style={{ textAlign: 'center', padding: '12px 0', color: '#999', fontSize: '0.82rem' }}
              >
                No rows — press + to add
              </td>
            </tr>
          ) : (
            rows.map((row, ri) => (
              <Tr key={row.id} $selected={row.id === selectedId} onClick={() => setSelectedId(row.id)}>
                <Td style={{ fontWeight: 600, color: '#666', minWidth: 22 }}>{ri + 1}</Td>
                {[0, 1, 2].map((pi) => {
                  const pulse = row.pulses[pi]
                  const enabled = pulse !== null
                  const prevDefined = pi === 0 || row.pulses[pi - 1] !== null

                  return (
                    <React.Fragment key={pi}>
                      {pi > 0 && (
                        <TdSpacer>
                          {prevDefined && (
                            <ToggleBtn
                              $active={enabled}
                              title={enabled ? `Remove Pulse ${pi + 1}` : `Add Pulse ${pi + 1}`}
                              onClick={(e) => {
                                e.stopPropagation()
                                togglePulse(row.id, pi as 1 | 2)
                              }}
                            >
                              {enabled ? '−' : '+'}
                            </ToggleBtn>
                          )}
                        </TdSpacer>
                      )}
                      {FIELDS.map((f) => (
                        <Td key={`${pi}-${f.key}`}>
                          <CellInput
                            type="text"
                            inputMode="numeric"
                            disabled={!enabled}
                            value={enabled ? pulse[f.key] : ''}
                            $invalid={enabled ? isFieldInvalid(pulse, f.key) : false}
                            onChange={(e) => updateField(row.id, pi, f.key, e.target.value)}
                            onClick={(e) => e.stopPropagation()}
                          />
                        </Td>
                      ))}
                    </React.Fragment>
                  )
                })}
              </Tr>
            ))
          )}
        </tbody>
      </Table>
      <ButtonRow>
        <SmallBtn onClick={addRow}>+ Add row</SmallBtn>
        <RemoveBtn onClick={removeRow} disabled={selectedId === null}>
          − Remove
        </RemoveBtn>
        <Spacer />
        <SmallBtn onClick={handleSave} disabled={rows.length === 0}>
          Save as…
        </SmallBtn>
        <SmallBtn onClick={handleLoad}>Load</SmallBtn>
        <input
          ref={fileInputRef}
          type="file"
          accept=".json"
          style={{ display: 'none' }}
          onChange={handleFileInput}
        />
      </ButtonRow>
    </Wrapper>
  )
}
