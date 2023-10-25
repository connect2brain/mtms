import styled from 'styled-components'

export const StyledPanel = `
  padding: 25px 0px 40px 35px;
  border-radius: 5px;
  background-color: #f7f7f7;
  box-shadow: 0px 3px 6px rgba(0, 0, 0, 0.1);/
`

export const StyledButton = styled.button`
  width: 200px;
  height: 50px;

  font-size: 1.00rem;
  padding: 0.8rem 0.5rem;
  margin-bottom: 0.8rem;
  border: none;
  border-radius: 5px;
  background-color: #007BFF;
  color: white;
  cursor: pointer;

  &:hover {
    background-color: #0056b3;
  }
  &:disabled {
    background-color: #CCCCCC;
    color: #888888;
  }
  &:hover:disabled {
    background-color: #CCCCCC;
  }
`

export const StyledRedButton = styled(StyledButton)`
  background-color: #A00000;
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
