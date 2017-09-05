/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
/* global Event */

import { init } from '../helper/runtime'
import div from '../../../../render/vue/components/div'
import eventBubbleBundle from '../data/build/dotvue/event-bubble.js'

init('core node', (Vue, helper) => {
  const id = 'test-event-bubble'
  let vm = null
  let el = null
  before(() => {
    helper.register('div', div)
    vm = helper.createVm(eventBubbleBundle, id)
    el = vm.$el.querySelector('.event-bubble-outter')
  })

  describe('stop event bubble', function () {
    let infoStr = ''
    function trackerShouldBe (tracker, shouldBe) {
      shouldBe = infoStr + shouldBe
      infoStr = shouldBe
      expect(tracker).to.equal(shouldBe)
    }

    it('should trigger the closest parent.', function (done) {
      expect(vm.tracker).to.equal('')
      /**
       * click outter div. should trigget event on the outter div.
       * and should execute handlers by the priority of:
       * child vnode -> parent vnode.
       *  e.g.  div -> foo (whoes root element is the div.)
       */
      const evt = new Event('tap', { bubbles: true })
      evt._for = 'weex'
      el.dispatchEvent(evt)

      helper.registerDone(id, (tracker) => {
        trackerShouldBe(tracker, ' > in-bar-outter-div > component-bar')
        done()
      })
    })

    it('should not bubble if already triggered.', function (done) {
      const inner = vm.$el.querySelector('.event-bubble-inner')
      /**
       * click inner div. should just trigget the inner handler and
       * shouldn't bubbe to outter div.
       */
      const evt = new Event('tap', { bubbles: false })
      evt._for = 'weex'
      inner.dispatchEvent(evt)

      helper.registerDone(id, (tracker) => {
        trackerShouldBe(tracker, ' > in-bar-inner-div')
        helper.unregisterDone(id)
        done()
      })
    })
  })
})
