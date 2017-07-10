/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License") you may not use this file except in compliance
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
import {
  collectStatPerf,
  tagFirstScreen,
  tagImg,
  tagBeforeCreate,
  tagRootMounted,
  tagMounted,
  tagBeforeUpdate,
  tagUpdated,
  tagBegin,
  tagEnd
} from '../../../../render/vue/utils/perf'
describe('utils', function () {
  before(function () {})
  describe('pref', function () {
    before(function () {
      this.clock = sinon.useFakeTimers()
    })
    after(function () {
      this.clock.restore()
    })
    it('collectStatPerf', function () {
      expect(collectStatPerf).to.be.a('function')
      expect(collectStatPerf('totalTime', (new Date()).getTime())).to.be.a('undefined')
      expect(collectStatPerf()).to.be.a('undefined')
    })
    it('tagFirstScreen', function () {
      const weexEmit = sinon.stub(window.weex, 'emit')
      expect(tagFirstScreen).to.be.a('function')
      tagFirstScreen()
      expect(weexEmit.withArgs('renderfinish').callCount).to.be.equal(1)
      expect(weexEmit.withArgs('firstscreenfinish').callCount).to.be.equal(0)
      window._first_screen_detected = false
      tagFirstScreen()
      expect(weexEmit.withArgs('renderfinish').callCount).to.be.equal(2)
      expect(weexEmit.withArgs('firstscreenfinish').callCount).to.be.equal(1)
      weexEmit.restore()
    })
    it('tagImg', function () {
      const weexEmit = sinon.stub(window.weex, 'emit')
      tagImg()
      this.clock.tick(500)
      expect(weexEmit.withArgs('renderfinish').callCount).to.be.equal(1)
      expect(tagImg).to.be.a('function')
      weexEmit.restore()
    })
    it('tagBeforeCreate', function () {
      const shouldBe = tagBeforeCreate()
      this.clock.tick(25)
      expect(shouldBe).to.be.a('undefined')
      expect(tagBeforeCreate).to.be.a('function')
    })
    it('tagRootMounted', function () {
      expect(tagRootMounted()).to.be.a('undefined')
      expect(tagRootMounted).to.be.a('function')
    })
    it('tagMounted', function () {
      const tagmount = tagMounted()
      this.clock.tick(25)
      expect(tagMounted).to.be.a('function')
      expect(tagmount).to.be.a('undefined')
    })
    it('tagBeforeUpdate', function () {
      const tagbeforeupdate = tagBeforeUpdate()
      this.clock.tick(25)
      expect(tagBeforeUpdate).to.be.a('function')
      expect(tagbeforeupdate).to.be.a('undefined')
    })
    it('tagUpdated', function () {
      const tagupdated = tagUpdated()
      this.clock.tick(25)
      expect(tagUpdated).to.be.a('function')
      expect(tagupdated).to.be.a('undefined')
    })
    it('tagBegin', function () {
      const tagbegin = tagBegin()
      expect(tagbegin).to.be.a('undefined')
      expect(tagBegin).to.be.a('function')
    })
    it('tagEnd', function () {
      const tagend = tagEnd()
      expect(tagend).to.be.a('undefined')
      expect(tagEnd).to.be.a('function')
    })
  })
})
