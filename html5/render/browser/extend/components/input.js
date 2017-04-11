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
import { findEnterKeyType } from '../../utils/index'

let appendStyle

const availableTypes = ['text', 'password', 'tel', 'email', 'url', 'date', 'time']
const DEFAULT_TYPE = 'text'

function setPlaceholderColor (input, placeholderColor) {
  if (!placeholderColor) {
    return
  }
  const vendors = [
    '::-webkit-input-placeholder',
    ':-moz-placeholder',
    '::-moz-placeholder',
    ':-ms-input-placeholder',
    ':placeholder-shown'
  ]
  let css = ''
  const cssRule = 'color: ' + placeholderColor + ';'
  for (let i = 0, l = vendors.length; i < l; i++) {
    css += '.' + input.className + vendors[i] + '{'
           + cssRule + '}'
  }
  appendStyle(css, input.styleId, true)
}

const proto = {
  create () {
    const node = document.createElement('input')
    const uuid = Math.floor(10000000000000 * Math.random()) + Date.now()
    this.className = 'weex-ipt-' + uuid
    this.styleId = 'weex-style-' + uuid
    node.classList.add(this.className)
    node.classList.add('weex-element')
    this.placeholder && (node.placeholder = this.placeholder)
    this.createKeybordEvent(node)
    return node
  },

  // support enter key envent
  createKeybordEvent (node) {
    if (Array.isArray(this.data.event) && this.data.event.indexOf('return') > -1) {
      node.addEventListener('keyup', (ev) => {
        const code = ev.keyCode
        let key = ev.key
        if (code === 13) {
          if (key.toLowerCase() === 'tab') {
            key = 'next'
          }
          const rightKeyType = findEnterKeyType(this.data.attr['returnKeyType'])
          this.dispatchEvent('return', { returnKeyType: rightKeyType })
        }
      }, false)
    }
  },

  focus () {
    this.node.focus()
  },

  blur () {
    this.node.blur()
  }
}

// updatable attributes
const attr = {
  disabled (val) {
    this.node.disabled = !!val
  },

  placeholder (val) {
    this.node.placeholder = val || ''
  },

  value (val) {
    this.node.value = val || ''
  },

  autofocus (val) {
    this.node.autofocus = !!val
  },

  type (val) {
    this.node.type = availableTypes.indexOf(val) !== -1
      ? val
      : DEFAULT_TYPE
  },

  returnKeyType (val) {
    this.node.returnKeyType = val || ''
  }
}

// updatable styles
const style = {
  placeholderColor: function (val) {
    setPlaceholderColor(this, val)
  }
}

// events configurations
const event = {
  input: {
    updator () {
      return {
        attrs: {
          value: this.node.value
        }
      }
    },
    extra () {
      return {
        value: this.node.value,
        timestamp: Date.now()
      }
    }
  },

  change: {
    updator: function () {
      return {
        attrs: {
          value: this.node.value
        }
      }
    },
    extra: function () {
      return {
        value: this.node.value,
        timestamp: Date.now()
      }
    }
  },

  return: {
    updator: function (obj) {
      return {
        attrs: {
          value: this.node.value
        }
      }
    }
  }
}

function init (Weex) {
  const Atomic = Weex.Atomic
  const extend = Weex.utils.extend
  appendStyle = Weex.utils.appendStyle

  // attrs:
  //   - type: text|password|tel|email|url
  //   - value
  //   - placeholder
  //   - disabled
  //   - autofocus
  function Input (data) {
    Atomic.call(this, data)
  }
  Input.prototype = Object.create(Atomic.prototype)
  extend(Input.prototype, proto)
  extend(Input.prototype, { attr })
  extend(Input.prototype, {
    style: extend(Object.create(Atomic.prototype.style), style)
  })
  extend(Input.prototype, { event })

  Weex.registerComponent('input', Input)
}

export default { init }
