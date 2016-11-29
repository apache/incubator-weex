import { expect } from 'chai'
import * as validator from '../../../render/vue/utils/validator'

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

  it('isCSSColor (hash)', () => {
    const { isCSSColor } = validator

    expect(isCSSColor('#ABCDEF')).to.be.true
    expect(isCSSColor('#AB00EF')).to.be.true
    expect(isCSSColor('#ab00ef')).to.be.true
    expect(isCSSColor('#00CCEF')).to.be.true
    expect(isCSSColor('#A46cef')).to.be.true
    expect(isCSSColor('#352866')).to.be.true
    expect(isCSSColor('#000000')).to.be.true

    expect(isCSSColor('#1')).to.be.false
    expect(isCSSColor('#12')).to.be.false
    expect(isCSSColor('#123')).to.be.true
    expect(isCSSColor('#1234')).to.be.false
    expect(isCSSColor('#12345')).to.be.false
    expect(isCSSColor('#123456')).to.be.true
    expect(isCSSColor('#AA*BGG')).to.be.false
    expect(isCSSColor('#AABBGG')).to.be.false
    expect(isCSSColor('#AABBCCDD')).to.be.false
    expect(isCSSColor('#1234567')).to.be.false
    expect(isCSSColor('#12345678')).to.be.false
    expect(isCSSColor('#DCBE4FDC')).to.be.false
  })

  it('isCSSColor (name)', () => {
    const { isCSSColor } = validator

    expect(isCSSColor('red')).to.be.true
    expect(isCSSColor('RED')).to.be.true
    expect(isCSSColor('blue')).to.be.true
    expect(isCSSColor('salmon')).to.be.true
    expect(isCSSColor('unknown')).to.be.true

    expect(isCSSColor('+red')).to.be.false
    expect(isCSSColor('red & blue')).to.be.false
  })

  it('isCSSColor (rgb)', () => {
    const { isCSSColor } = validator

    expect(isCSSColor('rgb(0,0,0)')).to.be.true
    expect(isCSSColor('rgb (0,0,0)')).to.be.true
    expect(isCSSColor('rgb ( 0,0,0 )')).to.be.true
    expect(isCSSColor('rgb ( 0, 0, 0 )')).to.be.true
    expect(isCSSColor('rgb (256, 110, 24)')).to.be.true
    expect(isCSSColor('rgb (25.6, 11.0, .24)')).to.be.true

    expect(isCSSColor('rgb (256,, 24)')).to.be.false
    expect(isCSSColor('rgb (256, 0, 24,)')).to.be.false
    expect(isCSSColor('rgb (256, 0, 24, 0.2)')).to.be.false
  })

  it('isCSSColor (rgba)', () => {
    const { isCSSColor } = validator

    expect(isCSSColor('rgba(0,0,0,0)')).to.be.true
    expect(isCSSColor('rgba (0,0,0, 0)')).to.be.true
    expect(isCSSColor('rgba ( 0,0,0 , 0)')).to.be.true
    expect(isCSSColor('rgba ( 0, 0, 0 , 0)')).to.be.true
    expect(isCSSColor('rgba (256, 110, 24, 0.5)')).to.be.true
    expect(isCSSColor('rgba (25.6, 11.0, 24, .3)')).to.be.true

    expect(isCSSColor('rgba (256,, 24, 0)')).to.be.false
    expect(isCSSColor('rgba (256, 0, 24,, 0)')).to.be.false
    expect(isCSSColor('rgba (256, 0, 24, 0.2, 0)')).to.be.false
  })
})
