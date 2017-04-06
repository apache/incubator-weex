/* global Vue */

import './wx-env'
import * as utils from '../utils'

const weexModules = {}

const weex = {
  __vue__: null,
  utils,
  // units: window.CSS_UNIT,
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

; ['on', 'once', 'off', 'emit'].forEach(function (method) {
  weex[method] = function (...args) {
    if (!this._vue) {
      this._vue = new Vue()
    }
    return this._vue[`$${method}`](...args)
  }
})

export default weex
