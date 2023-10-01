import styled from 'styled-components'

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
