import semver from 'semver'
import { init } from '../helper'
import text from '../../../../render/vue/components/text'

init('<text> component', (Vue, helper) => {
  const { utils, compile } = helper

  before(() => {
    helper.register('text', text)
  })

  it('create simple text component', () => {
    const vm = compile(`<text>abc</text>`)

    expect(vm.$el.tagName).to.be.equal('P')
    expect(vm.$el.innerHTML).to.be.equal('abc')
  })

  it('empty text component', () => {
    const vm = compile(`<text></text>`)

    expect(vm.$el.tagName).to.be.equal('P')
    expect(vm.$el.innerHTML).to.be.equal('')
  })

  it('lines style', () => {
    const vm = compile(`<text style="lines:5;">abc</text>`)
    if (semver.gt(Vue.version, '2.0.8')) {
      expect(vm.$el.style.overflow).to.be.equal('hidden')
      expect(vm.$el.style.textOverflow).to.be.equal('ellipsis')
      expect(vm.$el.style.webkitLineClamp).to.be.equal('5')
    }
  })

  it('inline styles', () => {
    const vm = compile(`<text style="color:blue">abc</text>`)

    if (semver.gt(Vue.version, '2.0.8')) {
      expect(vm.$el.style.color).to.be.equal('blue')
    }
  })

  // Not sure about this feature.
  it('class property', () => {
    const vm = compile(`<text class="title"></text>`)
    expect(utils.toArray(vm.$el.classList)).to.include.members(['weex-text', 'weex-el', 'title'])
  })

  it('value property', () => {
    const vm = compile(`<text value="A"></text>`)
    expect(vm.$el.innerHTML).to.be.equal('A')
  })

  it('both have value and content', () => {
    const vm = compile(`<text value="A">B</text>`)
    expect(vm.$el.innerHTML).to.be.equal('B')
  })

  // describe.skip('error usage (on native)', () => {
  //   it('contain other tags', () => {
  //     const vm = compile(`<text><b>abc</b></text>`)
  //     const span = vm.$el.children[0]

  //     expect(span.tagName).to.be.equal('SPAN')
  //     expect(span.innerHTML).to.be.equal('')
  //   })
  // })
})
