console.log(Cypress.env())
const testUrl = `http://${Cypress.env('BASE_URL') ?? 'localhost:3000'}`

describe('Note app', () => {
  beforeEach(() => {
    cy.visit(testUrl + '/targets')
  })

  it('front page can be opened', () => {
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

        cy.get('#add-target-button').click()

        cy.get('#targets-table')
          .find('tr')
          .then((newRows) => {
            expect(newRows.toArray().length).to.equal(originalLength + 1)
          })
      })
  })
})
