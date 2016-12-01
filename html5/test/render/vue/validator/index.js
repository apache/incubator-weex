import { expect } from 'chai'
import * as validator from '../../../../render/vue/validator/index'

describe('validator', function () {
  it('validateStyles', () => {
    const { validateStyles } = validator

    const styles = {
      position: 'absolute',
      color: '#dddddd'
    }

    expect(validateStyles).to.be.a('function')
    expect(validateStyles('text', styles)).to.be.true
    expect(validateStyles('text', { opacity: 0.2 })).to.be.true
  })

  it('validateStyles (invalid)', () => {
    const { validateStyles } = validator

    expect(validateStyles).to.be.a('function')
    expect(validateStyles('unknown', { color: 'blue' })).to.be.true
    expect(validateStyles('text', { unknown: 'nothing' })).to.be.false
    expect(validateStyles('text', { color: '#ABCC' })).to.be.false
  })
})
