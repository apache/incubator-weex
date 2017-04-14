import { init } from '../helper/runtime'
import div from '../../../../render/vue/components/div'

import eventBubbleBundle from '../data/build/dotvue/event-bubble.js'

init('core node', (Vue, helper) => {
  const id = 'test-event-bubble'

  before(() => {
    helper.register('div', div)
  })
  
  describe('stop event bubble', function () {
    let infoStr = ''
    let i = 0
    function trackerShouldBe (tracker, shouldBe) {
      shouldBe = infoStr + shouldBe
      expect(tracker).to.equal(shouldBe)
      infoStr = shouldBe
    }
    
    it('should trigger the closest parent.', function (done) {
      const vm = helper.createVm(eventBubbleBundle, id)
      const el = vm.$el.querySelector('.event-bubble-outter')
      expect(vm.tracker).to.equal('')
      
      /**
       * click outter div. should trigget event on the outter div.
       * and should execute handlers by the priority of:
       * child vnode -> parent vnode.
       *  e.g.  div -> foo (whoes root element is the div.)
       */
      const evt = new Event('click', { bubbles: true })
      el.dispatchEvent(evt)

      helper.registerDone(id, (tracker) => {
        trackerShouldBe(tracker, ' > in-bar-outter-div > component-bar')
        helper.unregisterDone(id)
        done()
      })
    })

    it ('should not bubble if already triggered.', function (done) {

      const vm = helper.createVm(eventBubbleBundle, id)
      const inner = vm.$el.querySelector('.event-bubble-inner')

      /**
       * click inner div. should just trigget the inner handler and
       * shouldn't bubbe to outter div.
       */
      const evt = new Event('click', { bubbles: true })
      inner.dispatchEvent(evt)

      helper.registerDone(id, (tracker) => {
        trackerShouldBe(tracker, ' > in-bar-inner-div')
        helper.unregisterDone(id)
        done()
      })
    })
  })
})
