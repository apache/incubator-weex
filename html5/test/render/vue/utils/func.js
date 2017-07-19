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
import * as utils from '../../../../render/vue/utils/func'
describe('utils', function () {
  describe('function', function () {
    before(() => {
      this.clock = sinon.useFakeTimers()
    })
    after(() => {
      this.clock.restore()
    })
    it('extend', () => {
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
    it('extendTruthy', () => {
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
    it('extendKeys', () => {
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
    it('extractKeys', () => {
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
    it('bind', () => {
      const {
        bind
      } = utils
      const testfn = function (name) {
        if (name) {
          this.name = name
        }
        return this.name ? this.name : ''
      }
      function TEST () {
        this.name = 'test'
      }
      const shouldBe = bind(testfn, new TEST())()
      expect(bind).to.be.a('function')
      expect(shouldBe).to.be.equal('test')
    })
    it('debounce', () => {
      const {
        debounce
      } = utils
      const shouldBe = 'test'
      let expected
      debounce(() => {
        expected = shouldBe
      }, 500)()
      this.clock.tick(500)
      expect(shouldBe).to.be.equal(expected)
      expect(debounce).to.be.a('function')
    })
    it('depress', () => {
      const {
        depress
      } = utils
      const shouldBe = 'test'
      let expected
      depress(() => {
        expected = shouldBe
      }, 100)()
      this.clock.tick(100)
      expect(shouldBe).to.be.equal(expected)
      expect(depress).to.be.a('function')
    })
    it('throttle', () => {
      const {
        throttle
      } = utils
      let expected
      const wait = 100
      const throttlecb = throttle(function (parmas) {
        expected = parmas
      }, wait, true)
      throttlecb('test1')
      setTimeout(() => {
        throttlecb('test2')
      }, 50)
      this.clock.tick(wait)
      this.clock.tick(50)
      this.clock.tick(wait + (wait > 25 ? wait : 25))
      expect(expected).to.be.equal('test1')
      expect(throttle).to.be.a('function')
    })
    it('loopArray', () => {
      const {
        loopArray
      } = utils
      expect(loopArray).to.be.a('function')
      expect(loopArray()).to.be.equal(undefined)
      expect(loopArray([1, 2, 3], 1)).to.be.deep.equal([3, 1, 2])
      expect(loopArray([1, 2, 3], -1)).to.be.deep.equal([2, 3, 1])
      expect(loopArray([1, 2, 3], 0)).to.be.deep.equal([1, 2, 3])
    })
    it('cached', () => {
      expect(utils.cached).to.be.a('function')
    })
    it('camelize', () => {
      const {
        camelize
      } = utils
      expect(camelize).to.be.a('function')
      expect(camelize('')).to.be.equal('')
      expect(camelize('dispaly')).to.be.equal('dispaly')
      expect(camelize('-webkit-transform')).to.be.equal('WebkitTransform')
      expect(camelize('text-overflow')).to.be.equal('textOverflow')
      expect(camelize('a-b-c-d')).to.be.equal('aBCD')
    })
    it('camelizeKeys', () => {
      const {
        camelizeKeys
      } = utils
      const camelizeobj = {
        dispaly: 'none',
        '-webkit-transform': 'all 1s ease'
      }
      const shouldBe = camelizeKeys(camelizeobj)
      const expected = {
        dispaly: 'none',
        WebkitTransform: 'all 1s ease'
      }
      expect(camelizeKeys).to.be.a('function')
      expect(shouldBe).to.be.deep.equal(expected)
    })
    it('capitalize', () => {
      const {
        capitalize
      } = utils
      expect(capitalize).to.be.a('function')
      expect(capitalize('')).to.be.equal('')
      expect(capitalize('string')).to.be.equal('String')
      expect(capitalize('string object')).to.be.equal('String object')
      expect(capitalize('[string object]')).to.be.equal('[string object]')
      expect(capitalize('I have an apple')).to.be.equal('I have an apple')
    })
    it('hyphenate', () => {
      const {
        hyphenate
      } = utils
      expect(hyphenate).to.be.a('function')
      expect(hyphenate('')).to.be.equal('')
      expect(hyphenate('dispaly')).to.be.equal('dispaly')
      expect(hyphenate('WebkitTransform')).to.be.equal('webkit-transform')
      expect(hyphenate('textOverflow')).to.be.equal('text-overflow')
      expect(hyphenate('aBCD')).to.be.equal('a-b-c-d')
    })
    it('hyphenateKeys', () => {
      const {
        hyphenateKeys
      } = utils
      const hyphenateobj = {
        dispaly: 'none',
        WebkitTransform: 'all 1s ease'
      }
      const shouldBe = hyphenateKeys(hyphenateobj)
      const expected = {
        dispaly: 'none',
        'webkit-transform': 'all 1s ease'
      }
      expect(hyphenateKeys).to.be.a('function')
      expect(shouldBe).to.be.deep.equal(expected)
    })
    it('hyphenateStyleKeys', () => {
      const {
        hyphenateStyleKeys
      } = utils
      const hyphenateStyleObj = {
        dispaly: 'none',
        'webkit-transform': 'all 1s ease'
      }
      const shouldBe = hyphenateStyleKeys(hyphenateStyleObj)
      const expected = {
        dispaly: 'none',
        '-webkit-transform': 'all 1s ease'
      }
      expect(hyphenateStyleKeys).to.be.a('function')
      expect(shouldBe).to.be.deep.equal(expected)
    })
    it('camelToKebab', () => {
      const {
        camelToKebab
      } = utils
      expect(camelToKebab).to.be.a('function')
      expect(camelToKebab('')).to.be.equal('')
      expect(camelToKebab('ABC')).to.be.equal('-a-b-c')
    })
    it('appendCss', () => {
      const {
        appendCss
      } = utils
      const cssid = 'append-test'
      const css = '.test{font-size:12px}'
      expect(appendCss).to.be.a('function')
      appendCss(css, cssid, true)
      let handler = document.querySelector('#' + cssid)
      expect(handler.textContent).to.be.equal(css)
      const anothercss = '.test2{font-size:16px}'
      appendCss(anothercss, cssid, true)
      handler = document.querySelector('#' + cssid)
      expect(handler.textContent).to.be.equal(anothercss)
    })
    it('nextFrame', done => {
      const {
        nextFrame
      } = utils
      let shouldBe
      const expected = 'test'
      const cb = () => {
        shouldBe = expected
      }
      expect(nextFrame).to.be.a('function')
      this.clock.restore()
      nextFrame(cb)
      setTimeout(() => {
        expect(shouldBe).to.be.equal(expected)
        done()
      }, 10)
    })
    it('toCSSText', () => {
      const {
        toCSSText
      } = utils
      const expected = 'font-size:12px;'
      const shouldBe = toCSSText({
        'font-size': '12px'
      })
      expect(toCSSText).to.be.a('function')
      expect(shouldBe).to.be.equal(expected)
    })
  })
})
