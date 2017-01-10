import * as utils from '../utils'
import { requireWeexModule } from '../modules'
export { utils }

const weexModules = {}

export function requireModule (moduleName) {
  const module = requireWeexModule(moduleName)
  if (module) {
    return module
  }
  return weexModules[moduleName]
}

// @deprecated
export function require (...args) {
  console.log(`[Vue Render] "weex.require" is deprecated, please use "weex.requireModule" instead.`)
  return requireModule(...args)
}

// @deprecated
// TODO: rename to registerModule
export function registerApiModule (name, module, meta) {
  if (!weexModules[name]) {
    weexModules[name] = {}
  }
  for (const key in module) {
    if (module.hasOwnProperty(key)) {
      weexModules[name][key] = utils.bind(module[key], this)
    }
  }
}

export function registerModule (...args) {
  return registerApiModule(...args)
}

export const sender = {
  performCallback (callback, data, keepAlive) {
    if (typeof callback === 'function') {
      return callback(data)
    }
    return null
  }
}

export function getRoot () {
}

export function install (module) {
  module.init(this)
}
