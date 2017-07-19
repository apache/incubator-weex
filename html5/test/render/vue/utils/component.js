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
/*global Event*/
import { init } from '../helper/runtime'
import div from '../../../../render/vue/components/div'

import firstScreenAppearBundle from '../data/build/dotvue/first-screen-appear.js'
import * as components from '../../../../render/vue/utils/component'

init('utils component', (Vue, helper) => {
  const spys = {
    'appear': sinon.spy(),
    'disappear': sinon.spy()
  }
  window._spy_first_screen_appear = function (evt) {
    const spy = spys[evt.type]
    spy && spy(evt)
  }

  const id = 'test-first-screen-appear'
  let vm = null

  before(() => {
    helper.register('div', div)
    vm = helper.createVm(firstScreenAppearBundle, id)
    vm.$el.style.height = '100px'
  })
  describe('component functions', () => {
    it('getParentScroller', () => {
      const { getParentScroller } = components
      expect(getParentScroller).to.be.a('function')
      expect(getParentScroller(vm)).to.be.a('undefined')
    })
    it('isComponentVisible', () => {
      const { isComponentVisible } = components
      expect(isComponentVisible).to.be.a('function')
      expect(isComponentVisible(vm)).to.be.true
    })
  })
  describe('watchAppear', () => {
    it('should work when mounted and updated.', function (done) {
      helper.registerDone(id, () => {
        const { appear: appearSpy, disappear: disappearSpy } = spys
        expect(appearSpy.callCount).to.equal(2)
        expect(disappearSpy.callCount).to.equal(1)
        expect(appearSpy.args[0][0].type).to.equal('appear')
        expect(appearSpy.args[1][0].type).to.equal('appear')
        expect(disappearSpy.args[0][0].type).to.equal('disappear')
        expect(appearSpy.args[0][0].direction).to.not.exist
        expect(appearSpy.args[1][0].direction).to.not.exist
        expect(disappearSpy.args[0][0].direction).to.not.exist
        window.dispatchEvent(new Event('scroll'))
        vm.$el.dispatchEvent(new Event('touchstart'))
        vm.$el.dispatchEvent(new Event('touchmove'))
        vm.$el.dispatchEvent(new Event('touchend'))
        vm.$el.dispatchEvent(new Event('touchcancel'))
        setTimeout(() => {
          helper.unregisterDone(id)
          done()
        }, 25)
      })
    })
  })
})
