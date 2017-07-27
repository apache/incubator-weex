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
  fireLazyload,
  getThrottleLazyload
} from '../../../../render/vue/utils/lazyload'
describe('utils', function () {
  describe('lazyload', function () {
    const validImageTransparent = 'data:image/gif;base64,R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7'
    const validImageBlack = 'data:image/gif;base64,R0lGODlhAQABAIAAAAUEBAAAACwAAAAAAQABAAACAkQBADs='
    const invalidImage = 'data:image/jpeg;base64,'
    before(() => {
      // for test collectStat in utils/perf.js
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
      this.weexEmit = sinon.stub(window.weex, 'emit')
    })
    after(() => {
      this.weexEmit.restore()
    })
    it('fireLazyload', (done) => {
      const node = document.createElement('figure')
      const urlReg = /http(s)?:\/\/(\S+):(\d+)\//
      const IMG_REC_INDENT = 500
      node.setAttribute('img-src', invalidImage)
      node.setAttribute('img-placeholder', validImageBlack)
      node.style.height = '10px'
        //  coverage branch if (item._src_loading)
      node._src_loading = true
        //  coverage branch if (Array.isArray(el))
      fireLazyload([node])
      node._src_loading = false
      fireLazyload(node, true)
      setTimeout(() => {
        expect(node.style.backgroundImage.replace(urlReg, '')).to.be.equal('url(' + validImageBlack + ')')
        setTimeout(() => {
          expect(this.weexEmit.withArgs('renderfinish').callCount).to.be.equal(1)
          expect(this.weexEmit.withArgs('firstscreenfinish').callCount).to.be.equal(0)
          done()
          document.body.removeChild(node)
        }, IMG_REC_INDENT)
      }, 100)
    })
    describe('getThrottleLazyload', () => {
      it('should use default value while parmas is undefined', (done) => {
        const IMG_REC_INDENT = 500
        window._first_screen_detected = true
        getThrottleLazyload()()
        setTimeout(() => {
          setTimeout(() => {
            expect(this.weexEmit.withArgs('renderfinish').callCount).to.be.equal(1)
            expect(this.weexEmit.withArgs('firstscreenfinish').callCount).to.be.equal(0)
            done()
          }, IMG_REC_INDENT)
        }, 16)
      })
      it('should be work', (done) => {
        const node = document.createElement('figure')
        const urlReg = /http(s)?:\/\/(\S+):(\d+)\//
        const wait = 100
        const IMG_REC_INDENT = 500
        node.style.height = '10px'
        node.setAttribute('img-src', validImageTransparent)
        node.setAttribute('img-placeholder', validImageBlack)
        document.body.appendChild(node)
        window._first_screen_detected = false
        getThrottleLazyload(wait, node)()
        setTimeout(() => {
          expect(node.style.backgroundImage.replace(urlReg, '')).to.be.equal('url(' + validImageTransparent + ')')
          setTimeout(() => {
            expect(this.weexEmit.withArgs('renderfinish').callCount).to.be.equal(2)
            expect(this.weexEmit.withArgs('firstscreenfinish').callCount).to.be.equal(1)
            done()
            document.body.removeChild(node)
          }, IMG_REC_INDENT)
        }, 100)
      })
    })
  })
})
