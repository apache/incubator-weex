import { expect } from 'chai'
import semver from 'semver'
import { multiDescribe } from '../helper'
import text from '../../../../render/vue/components/text'

multiDescribe('<text> component', (Vue, helper) => {
  before(() => {
    helper.register('text', text)
  })

  after(() => {
    helper.reset()
  })

  it('create simple text component', () => {
    const vm = helper.compile(`<text>abc</text>`)

    expect(vm.$el.tagName).to.be.equal('P')

    const span = vm.$el.children[0]
    expect(span.tagName).to.be.equal('SPAN')
    expect(span.innerHTML).to.be.equal('abc')
  })

  it('empty text component', () => {
    const vm = helper.compile(`<text></text>`)

    expect(vm.$el.tagName).to.be.equal('P')

    const span = vm.$el.children[0]
    expect(span.tagName).to.be.equal('SPAN')
    expect(span.innerHTML).to.be.equal('')
  })

  it('default styles', () => {
    const vm = helper.compile(`<text></text>`)
    const div = vm.$el
    const span = vm.$el.children[0]

    expect(div.className).to.be.equal('weex-container')
    expect(span.className).to.be.falsy

    // Only in Vue ^2.1.0 can get the style of elements
    if (semver.gt(Vue.version, '2.0.8')) {
      expect(div.style.fontSize).to.be.equal('32px')
      expect(span.style.whiteSpace).to.be.equal('pre-wrap')
      expect(span.style.wordWrap).to.be.equal('break-word')
      expect(span.style.display).to.be.equal('-webkit-box')
      expect(span.style.webkitBoxOrient).to.be.equal('vertical')
      expect(span.style.overflow).to.be.equal('visible')
      expect(span.style.textOverflow).to.be.equal('')
      expect(span.style.webkitLineClamp).to.be.equal('')
    }
  })

  it('lines property', () => {
    const vm = helper.compile(`<text lines="5">abc</text>`)
    const span = vm.$el.children[0]

    if (semver.gt(Vue.version, '2.0.8')) {
      expect(span.style.whiteSpace).to.be.equal('pre-wrap')
      expect(span.style.wordWrap).to.be.equal('break-word')
      expect(span.style.display).to.be.equal('-webkit-box')
      expect(span.style.webkitBoxOrient).to.be.equal('vertical')
      expect(span.style.overflow).to.be.equal('hidden')
      expect(span.style.textOverflow).to.be.equal('ellipsis')
      expect(span.style.webkitLineClamp).to.be.equal('5')
    }
  })

  it('inline styles', () => {
    const vm = helper.compile(`<text style="color:blue">abc</text>`)
    const div = vm.$el
    const span = vm.$el.children[0]

    if (semver.gt(Vue.version, '2.0.8')) {
      expect(div.style.color).to.be.equal('')
      expect(span.style.color).to.be.equal('blue')
      expect(span.style.whiteSpace).to.be.equal('pre-wrap')
      expect(span.style.wordWrap).to.be.equal('break-word')
      expect(span.style.display).to.be.equal('-webkit-box')
      expect(span.style.webkitBoxOrient).to.be.equal('vertical')
      expect(span.style.overflow).to.be.equal('visible')
      expect(span.style.textOverflow).to.be.equal('')
      expect(span.style.webkitLineClamp).to.be.equal('')
    }
  })

  it('override default styles', () => {
    const vm = helper.compile(`
      <text
        style="white-space:nowrap; color:blue; overflow: hidden;"
        >abc</text>
    `)
    const span = vm.$el.children[0]

    if (semver.gt(Vue.version, '2.0.8')) {
      expect(span.style.color).to.be.equal('blue')
      expect(span.style.whiteSpace).to.be.equal('nowrap')
      expect(span.style.wordWrap).to.be.equal('break-word')
      expect(span.style.display).to.be.equal('-webkit-box')
      expect(span.style.webkitBoxOrient).to.be.equal('vertical')
      expect(span.style.overflow).to.be.equal('hidden')
      expect(span.style.textOverflow).to.be.equal('')
      expect(span.style.webkitLineClamp).to.be.equal('')
    }
  })

  // Not sure about this feature.
  it('class property', () => {
    const vm = helper.compile(`<text class="title"></text>`)
    expect(vm.$el.className).to.be.equal('weex-container title')
    expect(vm.$el.children[0].className).to.be.equal('')
  })

  it('value property', () => {
    const vm = helper.compile(`<text value="A"></text>`)
    expect(vm.$el.children[0].innerHTML).to.be.equal('A')
  })

  it('both have value and content', () => {
    const vm = helper.compile(`<text value="A">B</text>`)
    expect(vm.$el.children[0].innerHTML).to.be.equal('B')
  })

  describe.skip('error usage (on native)', () => {
    it('contain other tags', () => {
      const vm = helper.compile(`<text><b>abc</b></text>`)
      const span = vm.$el.children[0]

      expect(span.tagName).to.be.equal('SPAN')
      expect(span.innerHTML).to.be.equal('')
    })
  })
})
