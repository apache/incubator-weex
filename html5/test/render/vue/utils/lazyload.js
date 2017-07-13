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
import * as lazyload from '../../../../render/vue/utils/lazyload'
describe('utils', function () {
  describe('lazyload', function () {
    before(() => {
      this.clock = sinon.useFakeTimers()
    })
    after(() => {
      this.clock.restore()
    })
    it('applySrc', () => {
      const {
        applySrc
      } = lazyload
      const node = document.createElement('figure')
      node.setAttribute('img-src', 'http://via.placeholder.com/1x1')
      node.setAttribute('img-placeholder', 'http://via.placeholder.com/2x2')
      document.body.appendChild(node)
      expect(applySrc(node, '')).to.be.a('undefined')
      expect(node.attributes[0].name).to.be.equal('img-src')
    })
    it('fireLazyload', () => {
      const {
        fireLazyload
      } = lazyload
      const node = document.createElement('figure')
      node.setAttribute('img-src', 'http://via.placeholder.com/1x1')
      node.setAttribute('img-placeholder', 'http://via.placeholder.com/2x2')
      document.body.appendChild(node)
      fireLazyload(node, true)
      expect(node.style.backgroundImage).to.be.equal('url(http://via.placeholder.com/1x1)')
    })
    it('getThrottleLazyload', () => {
      const {
        getThrottleLazyload
      } = lazyload
      const node = document.createElement('figure')
      node.setAttribute('img-src', 'http://via.placeholder.com/1x1')
      node.setAttribute('img-placeholder', 'http://via.placeholder.com/2x2')
      document.body.appendChild(node)
      const throttle = getThrottleLazyload(100, node)
      expect(throttle).to.be.a('function')
      throttle()
        //   this.clock.tick(100)
        //   console.log(node.style.backgroundImage)
        //   expect(node.style.backgroundImage).to.be.equal('url(http://localhost:9876/img-src.jpg)')
    })
  })
})
