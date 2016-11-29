import { expect } from 'chai'
import { multiDescribe } from '../utils'
import list from '../../../../render/vue/components/list.vue'

multiDescribe('<list> component', (Vue, helper) => {
  before(() => {
    helper.register('list', list)
  })

  after(() => {
    helper.reset()
  })

  it('create simple list component', () => {
    const vm = helper.compile(`<list><cell></cell></list>`)

    // console.log(vm.$el)
    expect(vm.$el.tagName).to.be.equal('DIV')
    expect(vm.$el.className).to.be.equal('weex-container scrollable-wrap list-wrap')
  })
})
