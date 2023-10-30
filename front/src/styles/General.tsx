import styled from 'styled-components'

export const Select = styled.select`
  width: 318px;
  padding: 8px;
  border: 1px solid #ccc;
  border-radius: 4px;
  outline: none;
  transition: background-color 0.2s;
  appearance: none;
  margin-right: 30px;

  &:focus {
    background-color: transparent;
  }
`

export const StyledPanel = styled.div<{ isGrayedOut?: boolean }>`
  padding: 25px 0px 40px 35px;
  border-radius: 5px;
  background-color: #f7f7f7;
  box-shadow: 0px 3px 6px rgba(0, 0, 0, 0.1);/
  ${({ isGrayedOut }) =>
    isGrayedOut &&
    `
    filter: grayscale(100%);
    color: #aaa;
    background-color: #f0f0f0;
    transition: filter 0.3s ease, opacity 0.3s ease;
    pointer-events: none;
  `}
`

export const StyledButton = styled.button`
  width: 200px;
  height: 50px;

  font-size: 1rem;
  padding: 0.8rem 0.5rem;
  margin-bottom: 0.8rem;
  border: none;
  border-radius: 5px;
  background-color: #007bff;
  color: white;
  cursor: pointer;

  &:hover {
    background-color: #0056b3;
  }
  &:disabled {
    background-color: #cccccc;
    color: #888888;
  }
  &:hover:disabled {
    background-color: #cccccc;
  }
`

export const StyledRedButton = styled(StyledButton)`
  background-color: #a00000;
  &:hover {
    background-color: #700000;
  }
`

export const TabBar = styled.div`
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

export const ProjectRow = styled.div`
  display: flex;
  justify-content: flex-start;
  align-items: center;
  gap: 10px;
  margin-bottom: 20px;
`

/* General config-related */
export const ConfigRow = styled.div`
  display: flex;
  justify-content: flex-start;
  align-items: center;
  gap: 10px;
  margin-bottom: 10px;
  padding-right: 0px;
`

export const CloseConfigRow = styled(ConfigRow)`
  margin-bottom: 8px;
`

export const ConfigLabel = styled.label`
  width: 300px;
  font-size: 14px;
`

export const IndentedLabel = styled(ConfigLabel)`
  padding-left: 15px;
`

/* For showing, e.g., session state. */
export const StateRow = styled.div`
  display: flex;
  justify-content: space-between;
  margin-bottom: 0.5rem;
  margin-right: 15px;
`

export const StateTitle = styled.span`
  font-weight: bold;
  color: #333;
  margin-right: 1rem;
`

export const IndentedStateTitle = styled(StateTitle)`
  margin-left: 20px;
`

export const StateValue = styled.span``
