import { expect } from 'chai'
import { multiDescribe } from '../utils'
import a from '../../../../render/vue/components/a.vue'

multiDescribe('a component', (Vue, helper) => {
  before(() => {
    Vue.config.isReservedTag = function () { return false }
    Vue.component('a', Vue.extend(a))
  })

  after(() => {
    Vue.component('a', null)
  })

  it('create simple a component', () => {
    const vm = helper.createVm(`<a href="http://dotwe.org">abc</a>`)

    // console.log(vm.$el)
    expect(vm.$el.tagName).to.be.equal('DIV')
    expect(vm.$el.className).to.be.equal('weex-container')
    expect(vm.$el.innerHTML).to.be.equal('<span>abc</span>')
  })
})
