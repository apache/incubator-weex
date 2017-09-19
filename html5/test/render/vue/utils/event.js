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
import * as event from '../../../../render/vue/utils/event'
describe('utils', function () {
  describe('event', function () {
    it('createEvent', () => {
      const {
        createEvent
      } = event
      const clickEvent = createEvent('', 'click')
      expect(createEvent).to.be.a('function')
      expect(clickEvent.type).to.be.equal('click')
      expect(clickEvent.target).to.be.a('null')
    })
    it('createBubblesEvent', () => {
      const {
        createBubblesEvent
      } = event
      const clickEvent = createBubblesEvent('', 'click')
      expect(createBubblesEvent).to.be.a('function')
      expect(clickEvent.type).to.be.equal('click')
      expect(clickEvent.target).to.be.a('null')
      expect(clickEvent.bubbles).to.be.true
    })
    it('createCustomEvent', () => {
      const {
        createCustomEvent
      } = event
      const customEvent = createCustomEvent('', 'customEvent')
      expect(createCustomEvent).to.be.a('function')
      expect(customEvent.type).to.be.equal('customEvent')
      expect(customEvent.target).to.be.a('null')
    })
    it('dispatchEvent', (done) => {
      const {
        dispatchEvent
      } = event
      const node = document.createElement('div')
      let expected
      const shouldBe = 'test'
      document.body.appendChild(node)
      node.addEventListener('click', () => {
        expected = shouldBe
        document.body.removeChild(node)
        expect(expected).to.be.equal(shouldBe)
        done()
      })
      const clickevent = document.createEvent('HTMLEvents')
      clickevent.initEvent('click', false, true)
      dispatchEvent(node, clickevent)
      expected(dispatchEvent).to.be.a('function')
    })
    it('mapFormEvents', () => {
      const {
        mapFormEvents
      } = event
      const context = {
        $el: {
          value: 'test'
        },
        $emit: () => {}
      }
      const spy = sinon.spy(context, '$emit')
      const map = mapFormEvents(context)
      const events = {
        value: ''
      }
      map.input(events)
      expect(spy.callCount).to.equal(1)
    })
  })
})
