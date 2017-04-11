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
'use strict'

const DEFAULT_LOAD_MORE_OFFSET = 0
let Component

const proto = {
  create (nodeType) {
    const node = document.createElement(nodeType || 'div')
    node.classList.add('weex-droot')
    node.classList.add('weex-container')
    return node
  },

  bindEvents (evts) {
    Component.prototype.bindEvents.call(this, evts)
    window.addEventListener('scroll', function (e) {
      // fire loadmore event.
      const body = document.body
      const leftDist = body.scrollHeight - window.innerHeight - body.scrollTop
      if (leftDist <= this.loadmoreoffset && this.isAvailableToFireloadmore) {
        this.isAvailableToFireloadmore = false
        this.dispatchEvent('loadmore')
      }
      else if (leftDist > this.loadmoreoffset && !this.isAvailableToFireloadmore) {
        this.isAvailableToFireloadmore = true
      }
    }.bind(this))
  }
}

const attr = {
  loadmoreoffset (val) {
    const value = parseFloat(val)
    if (value < 0 || isNaN(value)) {
      console.warn(`[h5-render] invalid loadmoreoffset: ${val}`)
      return
    }
    this.loadmoreoffset = value
  }
}

function init (Weex) {
  Component = Weex.Component
  const extend = Weex.utils.extend

  function Droot (data, nodeType) {
    if (data.ref !== '_root') {
      return console.warn('[h5-render] droot can only be used as a downgraded root.')
    }
    this.loadmoreoffset = DEFAULT_LOAD_MORE_OFFSET
    this.isAvailableToFireloadmore = true
    Component.call(this, data, nodeType)
  }
  Droot.prototype = Object.create(Component.prototype)
  extend(Droot.prototype, proto)
  extend(Droot.prototype, { attr })

  Weex.registerComponent('droot', Droot)
}

export default { init }
