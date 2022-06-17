import theme from '../../src/styles/theme'
import { clearRosState } from '../../src/services/ros'

const notSelectedColor = theme.colors.white
const selectedColor = theme.colors.lightgray

const testUrl = `http://${Cypress.env('FRONT_URL') ?? 'localhost:3000'}`
describe('Target table', () => {
  beforeEach(() => {
    clearRosState()
    cy.visit(testUrl + '/targets')
  })

  it('can be opened', () => {
    cy.contains('mTMS control panel')
  })

  it('allows adding new targets', () => {
    cy.get('#add-target-button').click()
    cy.get('#add-target-button').click()
    cy.wait(200)
    cy.get('#targets-table')
      .find('tr')
      .then((rows) => {
        const originalLength = rows.toArray().length
        rows[0].style.backgroundColor
        cy.get('#add-target-button').click()

        cy.get('#targets-table')
          .find('tr')
          .then((newRows) => {
            expect(newRows.toArray().length).to.equal(originalLength + 1)
          })
      })
  })

  it('allows editing targets', () => {
    cy.get('#add-target-button').click()
    cy.wait(200)

    cy.get('#cell-container-0-name').find('.cell-value-container').first().dblclick()
    cy.get('#cell-container-0-name').find('input').clear().type('first target')
    cy.get('#cell-container-0-name').find('input').type('{enter}')
  })
})
