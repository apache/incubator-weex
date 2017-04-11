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

/* global Vue */

import './wx-env'
import * as utils from '../utils'

const weexModules = {}

const weex = {
  __vue__: null,
  utils,
  units: window.CSS_UNIT,
  config: {
    env: window.WXEnvironment,
    bundleUrl: location.href
  },

  requireModule (moduleName) {
    return weexModules[moduleName]
  },

  registerModule (...args) {
    return this.registerApiModule(...args)
  },

  // @deprecated
  require (...args) {
    console.log(`[Vue Render] "weex.require" is deprecated, please use "weex.requireModule" instead.`)
    return this.requireModule(...args)
  },

  // @deprecated
  // TODO: rename to registerModule
  registerApiModule (name, module, meta) {
    if (!weexModules[name]) {
      weexModules[name] = {}
    }
    for (const key in module) {
      if (module.hasOwnProperty(key)) {
        weexModules[name][key] = utils.bind(module[key], this)
      }
    }
  },

  registerComponent (name, component) {
    if (!this.__vue__) {
      return console.log('[Vue Render] Vue is not found. Please import Vue.js before register a component.')
    }
    this.__vue__.component(name, component)
  },

  // @deprecated
  getRoot () {},

  // @deprecated
  sender: {
    performCallback (callback, data, keepAlive) {
      if (typeof callback === 'function') {
        return callback(data)
      }
      return null
    }
  },

  // @deprecated
  install (module) {
    module.init(this)
  }
}

; ['on', 'once', 'off', 'emit'].forEach(function (method) {
  weex[method] = function (...args) {
    if (!this._vue) {
      this._vue = new Vue()
    }
    return this._vue[`$${method}`](...args)
  }
})

export default weex
