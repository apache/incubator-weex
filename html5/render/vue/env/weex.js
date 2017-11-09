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
const _roots = []

const weex = {
  __vue__: null,
  utils,
  // units: window.CSS_UNIT,
  config: {
    env: window.WXEnvironment,
    bundleUrl: location.href
  },

  _components: {},
  _modules: weexModules,

  _meta: {
    mounted: {},
    updated: {},
    destroyed: {},
    requiredModules: {},
    apiCalled: {},
    perf: {}
  },

  document: {
    body: {}
  },

  requireModule (moduleName) {
    const metaMod = weex._meta.requiredModules
    if (!metaMod[moduleName]) {
      metaMod[moduleName] = 0
    }
    metaMod[moduleName]++
    return weexModules[moduleName]
  },

  registerModule (...args) {
    return this.registerApiModule(...args)
  },

  support (feature = '') {
    const match = (feature + '').match(/@(component|module)\/(\w+)(.\w+)?/)
    if (match) {
      const type = match[1]
      const mod = match[2]
      let method = match[3]
      method = method && method.replace(/^\./, '')
      switch (type) {
        case 'component':
          return typeof this._components[mod] !== 'undefined'
        case 'module':
          const module = weexModules[mod]
          return module && method ? !!module[method] : !!module
      }
    }
    else {
      console.warn(`[vue-render] invalid argument for weex.support: ${feature}`)
      return null
    }
  },

  /**
   * Register a new vue instance in this weex instance. Put its root element into weex.document.body.children, so
   * that user can use weex.document.body to walk through all dom structures in all vue instances in the page.
   */
  registerVueInstance (instance) {
    if (!instance instanceof Vue) {
      if (process.env.NODE_ENV === 'development') {
        console.error(`[vue-render] registerVueInstance: invalid instance, not a vue instance.`)
      }
      return
    }
    const root = instance.$root
    if (!root || !root.$el) {
      if (process.env.NODE_ENV === 'development') {
        console.error(`[vue-render] registerVueInstance: instance has no root.`)
      }
      return
    }
    this.document.body.children.push(root.$el)
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
    if (!!meta && meta.mountType === 'full') {
      weexModules[name] = module
    }
    for (const key in module) {
      if (module.hasOwnProperty(key)) {
        weexModules[name][key] = function () {
          const called = weex._meta.apiCalled
          if (!called[name]) {
            called[name] = {}
          }
          const calledMod = called[name]
          if (!calledMod[key]) {
            calledMod[key] = 0
          }
          calledMod[key]++
          return module[key].apply(weex, arguments)
        }
      }
    }
  },

  registerComponent (name, component) {
    if (!this.__vue__) {
      return console.log('[Vue Render] Vue is not found. Please import Vue.js before register a component.')
    }
    this._components[name] = 0
    if (component._css) {
      const css = component._css.replace(/\b[+-]?[\d.]+rem;?\b/g, function (m) {
        return parseFloat(m) * 75 * weex.config.env.scale + 'px'
      })
      utils.appendCss(css, `weex-cmp-${name}`)
      delete component._css
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

Object.defineProperty(weex.document.body, 'children', {
  get () { return _roots }
})

; ['on', 'once', 'off', 'emit'].forEach(function (method) {
  weex[method] = function (...args) {
    if (!this._vue) {
      this._vue = new this.__vue__()
    }
    return this._vue[`$${method}`](...args)
  }
})

export default weex
