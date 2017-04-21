import { init } from '../helper'
import list from '../../../../render/vue/components/scrollable/list'
import cell from '../../../../render/vue/components/scrollable/list/cell'

init('<list> component', (Vue, helper) => {
  const { utils, compile } = helper

  before(() => {
    helper.register('list', list)
    helper.register('cell', cell)
  })

  it('create simple list component', () => {
    const vm = compile(`<list><cell></cell></list>`)
    expect(vm.$el.tagName.toLowerCase()).to.be.equal('main')
    expect(utils.toArray(vm.$el.classList)).to.include.members(['weex-list', 'weex-list-wrapper', 'weex-ct'])
  })
})
