import { init } from '../helper'
import list from '../../../../render/vue/components/scrollable/list'
import cell from '../../../../render/vue/components/scrollable/list/cell'
import text from '../../../../render/vue/components/text'

init('list & cell example', (Vue, helper) => {
  const { utils, createVm } = helper

  before(() => {
    helper.register('list', list)
    helper.register('cell', cell)
    helper.register('text', text)
  })

  it('create simple list component', () => {
    const vm = createVm({
      template:
      `<list>
        <cell v-for="char in lists">
          <text>-- {{char}} --</text>
        </cell>
      </list>`,
      data: function () {
        return {
          lists: ['A', 'B', 'C']
        }
      }
    })

    expect(vm.$el.tagName).to.be.equal('MAIN')
    expect(utils.toArray(vm.$el.classList)).to.include.members(['weex-list', 'weex-list-wrapper'])
  })
})
