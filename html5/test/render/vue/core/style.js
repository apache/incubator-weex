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
import { init } from '../helper/runtime'
import div from '../../../../render/vue/components/div'
import image from '../../../../render/vue/components/image'
import { init as initViewport } from '../../../../render/vue/env/viewport'

import scopedStyleBundle from '../data/build/dotvue/scoped-style'

// function toArray (arr) {
//   return Array.prototype.slice.call(arr)
// }

// function getVStyleSheetNodes () {
//   const regVStyleSheets = /((?:,?\s*\.[\w-]+\[data-v-\w+\](?::\w+)?)+)\s*({[^}]+)/
//   const nodes = toArray(document.styleSheets)
//     .filter(function (styleSheet) {
//       return regVStyleSheets.test(styleSheet.ownerNode.textContent)
//     }).map(function (styleSheet) {
//       return styleSheet.ownerNode
//     })
//   return nodes
// }

init('core style', (Vue, helper) => {
  const { scale } = initViewport()
  before(() => {
    helper.register('div', div)
    helper.register('image', image)
  })

  it('should get normalized merged styles.', function (done) {
    const vm = helper.createVm(scopedStyleBundle)
    const el = vm.$refs.foo.$el || vm.$refs.foo
    expect(el).to.be.ok
    const expectedMap = {
      width: 200 * scale + 'px',
      height: 200 * scale + 'px',
      backgroundColor: 'red'
    }
    const expectedDirection = {
      WebkitBoxDirection: 'normal',
      WebkitBoxOrient: 'horizontal',
      WebkitFlexDirection: 'row',
      flexDirection: 'row'
    }
    const expectedTransform = {
      WebkitTransform: `translate3d(${100 * scale + 'px'}, ${100 * scale + 'px'}, 0px)`,
      transform: `translate3d(${100 * scale + 'px'}, ${100 * scale + 'px'}, 0px)`
    }

    for (const k in expectedMap) {
      expect(el.style[k]).to.equal(expectedMap[k])
    }
    const directionRes = []
    for (const k in expectedDirection) {
      directionRes.push(el.style[k] === expectedDirection[k])
    }
    expect(directionRes).to.include(true)

    const transformRes = []
    for (const k in expectedTransform) {
      transformRes.push(el.style[k] === expectedTransform[k])
    }
    expect(transformRes).to.include(true)

    const id = 'test-style'
    helper.registerDone(id, () => {
      expect(el.style.backgroundImage).to.match(
        /(?:-webkit-|-moz-|-ms-|-o-)?linear-gradient\(to top, (?:rgb\(245, 254, 253\)|#f5fefd), (?:rgb\(255, 255, 255\)|#ffffff)\)/)
      expect(['-webkit-box',
        '-moz-box',
        '-ms-flexbox',
        '-webkit-flex',
        'flex']).to.include(el.style.display)
      helper.unregisterDone(id)
      done()
    })
  })
})
