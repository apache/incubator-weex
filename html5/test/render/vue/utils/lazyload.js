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
    const validImage_transparent = 'data:image/gif;base64,R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7'
    const validImage_black = 'data:image/gif;base64,R0lGODlhAQABAIAAAAUEBAAAACwAAAAAAQABAAACAkQBADs='
    const invalidImage = 'data:image/jpeg;base64,'
    before(() => {
    //   this.clock = sinon.useFakeTimers()
    })
    after(() => {
    //   this.clock.restore()
    })
    it('fireLazyload', () => {
      const {
        fireLazyload
      } = lazyload
      const node = document.createElement('figure')
      const urlReg = /http(s)?:\/\/(\S+):(\d+)\//
      node.style.height = '10px'
      node.setAttribute('img-src', validImage_transparent)
      node.setAttribute('img-placeholder', validImage_transparent)
      document.body.appendChild(node)
      fireLazyload([node])
      setTimeout(() => {
          console.log(node)
      },200)
      expect(node.style.backgroundImage.replace(urlReg,'')).to.be.equal('url('+validImage_transparent+')')
      node.setAttribute('img-src', validImage_black)
      fireLazyload(node, true)
      expect(node.style.backgroundImage.replace(urlReg,'')).to.be.equal('url('+validImage_black+')')
      document.body.removeChild(node)
    })
    it('getThrottleLazyload', () => {
      const {
        getThrottleLazyload
      } = lazyload
      const node = document.createElement('figure')
      const urlReg = /http(s)?:\/\/(\S+):(\d+)\//
      const wait = 100,duration = wait + (wait > 25 ? wait : 25)
      node.style.height = '10px'
      node.setAttribute('img-src', validImage_transparent)
      node.setAttribute('img-placeholder', validImage_transparent)
      document.body.appendChild(node)
      getThrottleLazyload(wait, node)()
      document.body.removeChild(node)
      expect(node.style.backgroundImage.replace(urlReg,'')).to.be.equal('url('+validImage_transparent+')')
    })
  })
})
