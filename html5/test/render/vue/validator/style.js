import { expect } from 'chai'
import * as style from '../../../../render/vue/validator/style'

describe('supported', () => {
  describe('isCSSColor', () => {
    const { isCSSColor } = style

    it('isCSSColor (hash)', () => {
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
      expect(isCSSColor('red')).to.be.true
      expect(isCSSColor('RED')).to.be.true
      expect(isCSSColor('blue')).to.be.true
      expect(isCSSColor('salmon')).to.be.true
      expect(isCSSColor('unknown')).to.be.true

      expect(isCSSColor('+red')).to.be.false
      expect(isCSSColor('red & blue')).to.be.false
    })

    it('isCSSColor (rgb)', () => {
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

  it('color', () => {
    expect(style.color('#ABCDEF')).to.be.true
    expect(style.color('#CIHDEG')).to.be.false
  })

  it('background-color', () => {
    expect(style.backgroundColor('#ABCDEF')).to.be.true
    expect(style.backgroundColor('#CIHDEE')).to.be.false
  })

  it('position', () => {
    expect(style.position('absolute')).to.be.true
    expect(style.position('relative')).to.be.true
    expect(style.position('solid')).to.be.false
  })

  it('opacity', () => {
    expect(style.opacity('0')).to.be.true
    expect(style.opacity('1')).to.be.true
    expect(style.opacity(0)).to.be.true
    expect(style.opacity(1)).to.be.true
    expect(style.opacity('0.32')).to.be.true
    expect(style.opacity('.32')).to.be.true
    expect(style.opacity(0.32)).to.be.true

    /* eslint-disable no-floating-decimal */
    expect(style.opacity(.32)).to.be.true
    expect(style.opacity(.000001)).to.be.true
    /* eslint-enable no-floating-decimal */

    expect(style.opacity('2')).to.be.false
    expect(style.opacity('-3')).to.be.false
    expect(style.opacity(2)).to.be.false
    expect(style.opacity(-3)).to.be.false
    expect(style.opacity(null)).to.be.false
    expect(style.opacity('fade')).to.be.false
  })
})
