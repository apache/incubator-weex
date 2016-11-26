import { expect } from 'chai'
import { multiDescribe } from '../utils'
import text from '../../../../render/vue/components/text.vue'

multiDescribe('text component', (Vue, helper) => {
  before(() => {
    Vue.config.isReservedTag = function () { return false }
    Vue.component('text', Vue.extend(text))
  })

  after(() => {
    Vue.component('text', null)
  })

  it('create simple text component', () => {
    const vm = helper.createVm(`<text>abc</text>`)

    // console.log(vm.$el)
    expect(vm.$el.tagName).to.be.equal('DIV')
    expect(vm.$el.className).to.be.equal('weex-container')
    // expect(vm.$el.innerHTML).to.be.equal('<span style="white-space: pre-wrap; word-wrap: break-word;">abc</span>')

    const span = vm.$el.children[0]
    expect(span.tagName).to.be.equal('SPAN')
    expect(span.innerHTML).to.be.equal('abc')
    expect(span.className).to.be.falsy

    // TODO: make the style check more reliable
    // expect(span.getAttribute('style')).to.be.equal('white-space: pre-wrap; word-wrap: break-word;')
  })

  it('empty text component', () => {
    const vm = helper.createVm(`<text></text>`)

    expect(vm.$el.tagName).to.be.equal('DIV')
    expect(vm.$el.className).to.be.equal('weex-container')

    const span = vm.$el.children[0]
    expect(span.tagName).to.be.equal('SPAN')
    expect(span.innerHTML).to.be.equal('')
    expect(span.className).to.be.falsy
  })
})
