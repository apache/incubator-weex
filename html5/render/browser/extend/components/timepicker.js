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

const proto = {
  create () {
    const node = document.createElement('input')
    const uuid = Math.floor(10000000000000 * Math.random()) + Date.now()
    this.className = 'weex-ipt-' + uuid
    this.styleId = 'weex-style-' + uuid
    node.classList.add(this.className)
    node.setAttribute('type', 'time')
    node.type = 'time'
    // For the consistency of input component's width.
    // The date and time type of input will have a bigger width
    // when the 'box-sizing' is not set to 'border-box'
    node.classList.add('weex-element')
    return node
  }
}

const attr = {
  disabled: function (val) {
    this.node.disabled = val && val !== 'false'
  }
}

function init (Weex) {
  const Atomic = Weex.Atomic
  const extend = Weex.utils.extend

  // attrs:
  //   - value
  //   - disabled
  function Timepicker (data) {
    Atomic.call(this, data)
  }
  Timepicker.prototype = Object.create(Atomic.prototype)
  extend(Timepicker.prototype, proto)
  extend(Timepicker.prototype, { attr })

  Weex.registerComponent('timepicker', Timepicker)
}

export default { init }
