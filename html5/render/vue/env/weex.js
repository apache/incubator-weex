import * as utils from '../utils'
export { utils }

const weexModules = {}

export function require (moduleName) {
  return weexModules[moduleName]
}

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
