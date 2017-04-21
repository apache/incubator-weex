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

const DEFAULT_ROWS = 2

/**
 * attr:
 * rows: default is 2.
 * disabled
 * placeholder
 */

const proto = {
  create () {
    const node = document.createElement('textarea')
    node.classList.add('weex-element')
    node.classList.add('weex-textarea')
    this.createKeyboardEvent(node)
    return node
  },

  // support enter key envent
  createKeyboardEvent (node) {
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
  }
}

// updatable attributes
const attr = {
  rows (val) {
    this.node.rows = val || DEFAULT_ROWS
  },
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
  returnKeyType (val) {
    this.node.returnKeyType = val || ''
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

  // attrs:
  //   - rows
  //   - disabled
  //   - placeholder
  function Textarea (data) {
    Atomic.call(this, data)
  }
  Textarea.prototype = Object.create(Atomic.prototype)
  extend(Textarea.prototype, proto)
  extend(Textarea.prototype, { attr })
  extend(Textarea.prototype, { event })

  Weex.registerComponent('textarea', Textarea)
}

export default { init }
