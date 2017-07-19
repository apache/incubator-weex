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
  tagEnd,
  perf
} from '../../../../render/vue/utils/perf'
describe('utils', function () {
  describe('pref', function () {
    before(() => {
      this.clock = sinon.useFakeTimers()
    })
    after(() => {
      this.clock.restore()
    })
    beforeEach(() => {
      this.weexEmit = sinon.stub(window.weex, 'emit')
    })
    afterEach(() => {
      this.weexEmit.restore()
    })
    it('collectStatPerf', () => {
      window.performance = {
        getEntries: () => {
          return [{
            entryType: 'resource',
            name: 'weex-invalid.jpg'
          }, {
            entryType: 'resource',
            name: 'weex-test.jpg'
          }]
        }
      }
      window.history.pushState(null, null, '?_wx_tpl=./weex-test.jpg')
      const time = (new Date()).getTime()
      collectStatPerf('totalTime', time)
      expect(collectStatPerf).to.be.a('function')
      expect(perf.stat['totalTime']).to.be.equal(time)
      expect(collectStatPerf()).to.be.a('undefined')
    })
    it('tagFirstScreen', () => {
      expect(tagFirstScreen).to.be.a('function')
      tagFirstScreen()
      expect(this.weexEmit.withArgs('renderfinish').callCount).to.be.equal(1)
      expect(this.weexEmit.withArgs('firstscreenfinish').callCount).to.be.equal(0)
      window._first_screen_detected = false
      tagFirstScreen()
      expect(this.weexEmit.withArgs('renderfinish').callCount).to.be.equal(2)
      expect(this.weexEmit.withArgs('firstscreenfinish').callCount).to.be.equal(1)
    })
    it('tagImg', () => {
      tagImg()
      this.clock.tick(500)
      expect(this.weexEmit.withArgs('renderfinish').callCount).to.be.equal(1)
      expect(tagImg).to.be.a('function')
    })
    it('tagBeforeCreate', () => {
      tagBeforeCreate()
      this.clock.tick(25)
      const time = perf.earliestBeforeCreates[perf.earliestBeforeCreates.length - 1]
      expect(time).to.be.a('number')
      expect(perf.earliestBeforeCreates).to.have.lengthOf(1)
      expect(tagBeforeCreate).to.be.a('function')
    })
    it('tagRootMounted', () => {
      tagRootMounted()
      const time = perf.latestMounts[perf.latestMounts.length - 1]
      expect(time).to.be.a('number')
      expect(perf.stat['totalTime']).to.be.equal(parseInt(time))
      expect(tagRootMounted).to.be.a('function')
    })
    it('tagMounted', () => {
      perf.firstAllMountedTime = 0
      tagMounted()
      this.clock.tick(25)
      expect(tagMounted).to.be.a('function')
      expect(perf.latestMounts).to.have.lengthOf(2)
      expect(perf.createTime).to.have.lengthOf(1)
      expect(perf.createTime[perf.createTime.length - 1]).to.have.all.keys('start', 'end', 'duration')
      expect(perf.firstAllMountedTime).to.be.above(0)
    })
    it('tagBeforeUpdate', () => {
      tagBeforeUpdate()
      this.clock.tick(25)
      expect(tagBeforeUpdate).to.be.a('function')
      expect(perf.earliestBeforeUpdates).to.have.lengthOf(1)
    })
    it('tagUpdated', () => {
      tagUpdated()
      this.clock.tick(25)
      expect(tagUpdated).to.be.a('function')
      expect(perf.latestUpdates).to.have.lengthOf(1)
      expect(perf.updateTime).to.have.lengthOf(1)
      expect(perf.updateTime[perf.updateTime.length - 1]).to.have.all.keys('start', 'end', 'duration')
    })
    it('tagBegin', () => {
      const name = 'test'
      tagBegin(name)
      expect(tagBegin).to.be.a('function')
    })
    it('tagEnd', () => {
      const name = 'test'
      tagEnd(name)
      expect(tagEnd).to.be.a('function')
      expect(perf.time[name]).to.be.a('number')
    })
  })
})
