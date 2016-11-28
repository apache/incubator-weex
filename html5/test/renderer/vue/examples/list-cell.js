import { expect } from 'chai'
import { multiDescribe } from '../utils'
import list from '../../../../render/vue/components/list.vue'
import cell from '../../../../render/vue/components/cell.vue'
import text from '../../../../render/vue/components/text.vue'

multiDescribe('list & cell example', (Vue, helper) => {
  before(() => {
    helper.register('list', list)
    helper.register('cell', cell)
    helper.register('text', text)
  })

  after(() => {
    helper.reset()
  })

  it('create simple list component', () => {
    const vm = helper.createVm({
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

    // console.log(vm.$el)
    expect(vm.$el.tagName).to.be.equal('DIV')
    expect(vm.$el.className).to.be.equal('weex-container scrollable-wrap list-wrap')

    expect(vm.$el.children.length).to.be.equal(3)
  })
})
