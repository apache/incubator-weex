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
import * as utils from '../../../../render/vue/utils/func'
describe('utils', function () {
  describe('function', function () {
    before(function () {
      this.clock = sinon.useFakeTimers()
    })
    after(function () {
      this.clock.restore()
    })
    it('extend', function () {
      const {
        extend
      } = utils
      expect(extend).to.be.a('function')
      const abc = {
        name: 'abc'
      }
      expect(extend(abc)).to.deep.equal(abc)
      expect(extend(abc, {
        name: 'x'
      }) === abc).to.be.true
      expect(abc).to.deep.equal({
        name: 'x'
      })
      expect(abc).to.deep.equal({
        name: 'x'
      })
      expect(extend(abc, {})).to.be.deep.equal(abc)
    })
    it('extendTruthy', function () {
      const {
        extendTruthy
      } = utils
      expect(extendTruthy).to.be.a('function')
      const from = {
        '': '',
        'undefined': 'undefined',
        0: '0',
        test: 'test'
      }
      expect(extendTruthy({}, [from])[0]).to.be.deep.equal(from)
      expect(extendTruthy({}, [])).to.be.deep.equal({})
    })
    it('extendKeys', function () {
      const {
        extendKeys
      } = utils
      expect(extendKeys).to.be.a('function')
      const from = {
        test1: 'test1',
        test2: 'test2'
      }
      const expected = {
        test1: 'test1'
      }
      expect(extendKeys({}, from, ['test1'])).to.be.deep.equal(expected)
    })
    it('extractKeys', function () {
      const {
        extractKeys
      } = utils
      expect(extractKeys).to.be.a('function')
      const from = {
        test1: 'test1',
        test2: 'test2'
      }
      const toExpected = {
        test1: 'test1'
      }
      const fromExpected = {
        test2: 'test2'
      }
      expect(extractKeys({}, from, ['test1'])).to.be.deep.equal(toExpected)
      expect(from).to.be.deep.equal(fromExpected)
      expect(extractKeys({}, {}, [])).to.be.deep.equal({})
    })
    it('bind', function () {
      const {
        bind
      } = utils
      const testfn = function (name) {
        if (name) {
          this.name = name
        }
        return this.name ? this.name : ''
      }
      const TEST = function () {
        this.name = 'test'
      }
      const shouldBe = bind(testfn, new TEST())()
      expect(bind).to.be.a('function')
      expect(shouldBe).to.be.equal('test')
    })
    it('debounce', function () {
      const {
        debounce
      } = utils
      const shouldBe = 'test'
      let expected
      debounce(function () {
        expected = shouldBe
      }, 500)()
      this.clock.tick(500)
      expect(shouldBe).to.be.equal(expected)
      expect(debounce).to.be.a('function')
    })
    it('depress', function () {
      const {
        depress
      } = utils
      const shouldBe = 'test'
      let expected
      depress(function () {
        expected = shouldBe
      }, 100)()
      this.clock.tick(100)
      expect(shouldBe).to.be.equal(expected)
      expect(depress).to.be.a('function')
    })
    it('throttle', function () {
      const {
        throttle
      } = utils
      let expected
      const wait = 100
      const throttlecb = throttle(function (parmas) {
        expected = parmas
      }, wait, true)
      throttlecb('test1')
      setTimeout(function () {
        throttlecb('test2')
      }, 50)
      this.clock.tick(wait)
      this.clock.tick(50)
      this.clock.tick(wait + (wait > 25 ? wait : 25))
      expect(expected).to.be.equal('test1')
      expect(throttle).to.be.a('function')
    })
    it('loopArray', function () {
      const {
        loopArray
      } = utils
      expect(loopArray).to.be.a('function')
      expect(loopArray()).to.be.equal(undefined)
      expect(loopArray([1, 2, 3], 1)).to.be.deep.equal([3, 1, 2])
      expect(loopArray([1, 2, 3], -1)).to.be.deep.equal([2, 3, 1])
      expect(loopArray([1, 2, 3], 0)).to.be.deep.equal([1, 2, 3])
    })
  })
})
