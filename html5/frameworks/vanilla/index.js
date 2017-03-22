
const instances = {}
const modules = {}
const components = {}

const renderer = {
  instances,
  modules,
  components
}

/**
 * Prepare framework config, basically about the virtual-DOM and JS bridge.
 * @param {object} cfg
 */
export function init (cfg) {
  renderer.Document = cfg.Document
  renderer.Element = cfg.Element
  renderer.Comment = cfg.Comment
  renderer.sendTasks = cfg.sendTasks
}

/**
 * Reset framework config and clear all registrations.
 */
export function reset () {
  clear(instances)
  clear(modules)
  clear(components)
  delete renderer.Document
  delete renderer.Element
  delete renderer.Comment
  delete renderer.sendTasks
}

/**
 * Delete all keys of an object.
 * @param {object} obj
 */
function clear (obj) {
  for (const key in obj) {
    delete obj[key]
  }
}

/**
 * Create an instance with id, code, config and external data.
 * @param {string} instanceId
 * @param {string} appCode
 * @param {object} config
 * @param {object} data
 * @param {object} env { info, config, services }
 */
export function createInstance (
  instanceId,
  appCode = '',
  config = {},
  data,
  env = {}
) {
  // Virtual-DOM object.
  const document = new renderer.Document(instanceId, config.bundleUrl)

  // All function/callback of parameters before sent to native
  // will be converted as an id. So `callbacks` is used to store
  // these real functions. When a callback invoked and won't be
  // called again, it should be removed from here automatically.
  const callbacks = []

  // The latest callback id, incremental.
  const callbackId = 1

  instances[instanceId] = {
    instanceId, config, data,
    document, callbacks, callbackId
  }

  // Prepare native module getter and HTML5 Timer APIs.
  const moduleGetter = genModuleGetter(instanceId)
  const timerAPIs = getInstanceTimer(instanceId, moduleGetter)

  // Prepare `weex` instance variable.
  const weexInstanceVar = {
    config,
    document,
    requireModule: moduleGetter
  }
  Object.freeze(weexInstanceVar)

  // The function which create a closure the JS Bundle will run in.
  // It will declare some instance variables like HTML5 Timer APIs etc.
  const instanceVars = Object.assign({
    weex: weexInstanceVar,
    document // alias for `weex.document`
  }, timerAPIs)

  // wrap IFFE and use strict mode
  appCode = `(function(global){"use strict";\n ${appCode} \n})(Object.create(this))`

  callFunction(instanceVars, appCode)

  // Send `createFinish` signal to native.
  renderer.sendTasks(instanceId + '', [{ module: 'dom', method: 'createFinish', args: [] }], -1)
}

/**
 * Destroy an instance with id. It will make sure all memory of
 * this instance released and no more leaks.
 * @param {string} instanceId
 */
export function destroyInstance (instanceId) {
  delete instances[instanceId]
}

/**
 * Refresh an instance with id and new top-level component data.
 * define all possible meaningful keys when instance created.
 * @param {string} instanceId
 * @param {object} data
 */
export function refreshInstance (instanceId, data) {
  // Finally `refreshFinish` signal needed.
  renderer.sendTasks(instanceId + '', [{ module: 'dom', method: 'refreshFinish', args: [] }], -1)
}

/**
 * Get the JSON object of the root element.
 * @param {string} instanceId
 */
export function getRoot (instanceId) {
  const instance = instances[instanceId]
  return instance.document.body.toJSON()
}

/**
 * Receive tasks from native. Generally there are two types of tasks:
 * 1. `fireEvent`: an device actions or user actions from native.
 * 2. `callback`: invoke function which sent to native as a parameter before.
 * @param {string} instanceId
 * @param {array}  tasks
 */
export function receiveTasks (instanceId, tasks) {
  const instance = instances[instanceId]
  const { callbacks, document } = instance
  tasks.forEach(task => {
    // `fireEvent` case: find the event target and fire.
    if (task.method === 'fireEvent') {
      const [nodeId, type, e, domChanges] = task.args
      const el = document.getRef(nodeId)
      document.fireEvent(el, type, e, domChanges)
    }
    // `callback` case: find the callback by id and call it.
    if (task.method === 'callback') {
      const [callbackId, data, ifKeepAlive] = task.args
      const callback = callbacks[callbackId]
      if (typeof callback === 'function') {
        callback(data)
        // Remove the callback from `callbacks` if it won't called again.
        if (typeof ifKeepAlive === 'undefined' || ifKeepAlive === false) {
          callbacks[callbackId] = undefined
        }
      }
    }
  })
  // Finally `updateFinish` signal needed.
  renderer.sendTasks(instanceId + '', [{ module: 'dom', method: 'updateFinish', args: [] }], -1)
}

/**
 * Register native modules information.
 * @param {object} newModules
 */
export function registerModules (newModules) {
  for (const name in newModules) {
    if (!modules[name]) {
      modules[name] = {}
    }
    newModules[name].forEach(method => {
      if (typeof method === 'string') {
        modules[name][method] = true
      }
      else {
        modules[name][method.name] = method.args
      }
    })
  }
}

/**
 * Register native components information.
 * @param {array} newComponents
 */
export function registerComponents (newComponents) {
  if (Array.isArray(newComponents)) {
    newComponents.forEach(component => {
      if (!component) {
        return
      }
      if (typeof component === 'string') {
        components[component] = true
      }
      else if (typeof component === 'object' && typeof component.type === 'string') {
        components[component.type] = component
      }
    })
  }
}

/**
 * Generate native module getter. Each native module has several
 * methods to call. And all the behaviors is instance-related. So
 * this getter will return a set of methods which additionally
 * send current instance id to native when called. Also the args
 * will be normalized into "safe" value. For example function arg
 * will be converted into a callback id.
 * @param  {string}  instanceId
 * @return {function}
 */
function genModuleGetter (instanceId) {
  const instance = instances[instanceId]
  return function (name) {
    const nativeModule = modules[name] || []
    const output = {}
    for (const methodName in nativeModule) {
      output[methodName] = (...args) => {
        const finalArgs = args.map(value => {
          return normalize(value, instance)
        })
        renderer.sendTasks(instanceId + '', [{ module: name, method: methodName, args: finalArgs }], -1)
      }
    }
    return output
  }
}

/**
 * Generate HTML5 Timer APIs. An important point is that the callback
 * will be converted into callback id when sent to native. So the
 * framework can make sure no side effect of the callback happened after
 * an instance destroyed.
 * @param  {[type]} instanceId   [description]
 * @param  {[type]} moduleGetter [description]
 * @return {[type]}              [description]
 */
function getInstanceTimer (instanceId, moduleGetter) {
  const instance = instances[instanceId]
  const timer = moduleGetter('timer')
  const timerAPIs = {
    setTimeout: (...args) => {
      const handler = function () {
        args[0](...args.slice(2))
      }
      timer.setTimeout(handler, args[1])
      return instance.callbackId.toString()
    },
    setInterval: (...args) => {
      const handler = function () {
        args[0](...args.slice(2))
      }
      timer.setInterval(handler, args[1])
      return instance.callbackId.toString()
    },
    clearTimeout: (n) => {
      timer.clearTimeout(n)
    },
    clearInterval: (n) => {
      timer.clearInterval(n)
    }
  }
  return timerAPIs
}

/**
 * Call a new function body with some global objects.
 * @param  {object} globalObjects
 * @param  {string} code
 * @return {any}
 */
function callFunction (globalObjects, body) {
  const globalKeys = []
  const globalValues = []
  for (const key in globalObjects) {
    globalKeys.push(key)
    globalValues.push(globalObjects[key])
  }
  globalKeys.push(body)

  const result = new Function(...globalKeys)
  return result(...globalValues)
}

/**
 * Convert all type of values into "safe" format to send to native.
 * 1. A `function` will be converted into callback id.
 * 2. An `Element` object will be converted into `ref`.
 * The `instance` param is used to generate callback id and store
 * function if necessary.
 * @param  {any}    v
 * @param  {object} instance
 * @return {any}
 */
function normalize (v, instance) {
  const type = typof(v)

  switch (type) {
    case 'undefined':
    case 'null':
      return ''
    case 'regexp':
      return v.toString()
    case 'date':
      return v.toISOString()
    case 'number':
    case 'string':
    case 'boolean':
    case 'array':
    case 'object':
      if (v instanceof renderer.Element) {
        return v.ref
      }
      return v
    case 'function':
      instance.callbacks[++instance.callbackId] = v
      return instance.callbackId.toString()
    default:
      return JSON.stringify(v)
  }
}

/**
 * Get the exact type of an object by `toString()`. For example call
 * `toString()` on an array will be returned `[object Array]`.
 * @param  {any}    v
 * @return {string}
 */
function typof (v) {
  const s = Object.prototype.toString.call(v)
  return s.substring(8, s.length - 1).toLowerCase()
}
