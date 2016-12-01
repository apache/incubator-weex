import { expect } from 'chai'
import { multiDescribe } from '../helper'
import a from '../../../../render/vue/components/a.vue'

multiDescribe('<a> component', (Vue, helper) => {
  before(() => {
    helper.register('a', a)
  })

  after(() => {
    helper.reset()
  })

  it('create simple a component', () => {
    const vm = helper.compile(`<a href="http://dotwe.org">abc</a>`)

    // console.log(vm.$el)
    expect(vm.$el.tagName).to.be.equal('DIV')
    expect(vm.$el.className).to.be.equal('weex-container')
    expect(vm.$el.innerHTML).to.be.equal('<span>abc</span>')
  })
})
