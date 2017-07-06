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
import * as utils from '../../../../render/vue/utils'
describe('utils', function () {
  describe('index', function () {
    it('cached', function () {
      expect(utils.cached).to.be.a('function')
    })
    it('camelize', function () {
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
    it('camelizeKeys', function () {
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
    it('capitalize', function () {
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
    it('hyphenate', function () {
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
    it('hyphenateKeys', function () {
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
    it('hyphenateStyleKeys', function () {
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
    it('camelToKebab', function () {
      const {
        camelToKebab
      } = utils
      expect(camelToKebab).to.be.a('function')
      expect(camelToKebab('')).to.be.equal('')
      expect(camelToKebab('ABC')).to.be.equal('-a-b-c')
    })
    it('appendCss', function () {
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
    it('nextFrame', function (done) {
      const {
        nextFrame
      } = utils
      let shouldBe
      const expected = 'test'
      const cb = function () {
        shouldBe = expected
      }
      expect(nextFrame).to.be.a('function')
      nextFrame(cb)
      setTimeout(() => {
        expect(shouldBe).to.be.equal(expected)
        done()
      }, 10)
    })
    it('toCSSText', function () {
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
