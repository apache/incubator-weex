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
import { init as initTaskHandler } from './task-center'
import { registerElement } from './vdom/element-types'
import { services, register, unregister } from './service'

let frameworks
let runtimeConfig

const versionRegExp = /^\s*\/\/ *(\{[^}]*\}) *\r?\n/

/**
 * Detect a JS Bundle code and make sure which framework it's based to. Each JS
 * Bundle should make sure that it starts with a line of JSON comment and is
 * more that one line.
 * @param  {string} code
 * @return {object}
 */
function checkVersion (code) {
  let info
  const result = versionRegExp.exec(code)
  if (result) {
    try {
      info = JSON.parse(result[1])
    }
    catch (e) {}
  }
  return info
}

function createServices (id, env, config) {
  // Init JavaScript services for this instance.
  const serviceMap = Object.create(null)
  serviceMap.service = Object.create(null)
  services.forEach(({ name, options }) => {
    if (process.env.NODE_ENV === 'development') {
      console.debug(`[JS Runtime] create service ${name}.`)
    }
    const create = options.create
    if (create) {
      const result = create(id, env, config)
      Object.assign(serviceMap.service, result)
      Object.assign(serviceMap, result.instance)
    }
  })
  delete serviceMap.service.instance
  Object.freeze(serviceMap.service)
  return serviceMap
}

const instanceMap = {}

/**
 * Check which framework a certain JS Bundle code based to. And create instance
 * by this framework.
 * @param {string} id
 * @param {string} code
 * @param {object} config
 * @param {object} data
 */
function createInstance (id, code, config, data) {
  let info = instanceMap[id]

  if (!info) {
    // Init instance info.
    info = checkVersion(code) || {}
    if (!frameworks[info.framework]) {
      info.framework = 'Weex'
    }

    // Init instance config.
    config = JSON.parse(JSON.stringify(config || {}))
    config.bundleVersion = info.version
    config.env = JSON.parse(JSON.stringify(global.WXEnvironment || {}))
    console.debug(`[JS Framework] create an ${info.framework}@${config.bundleVersion} instance from ${config.bundleVersion}`)

    const env = {
      info,
      config,
      created: Date.now(),
      framework: info.framework
    }
    env.services = createServices(id, env, runtimeConfig)
    instanceMap[id] = env

    return frameworks[info.framework].createInstance(id, code, config, data, env)
  }
  return new Error(`invalid instance id "${id}"`)
}

const methods = {
  createInstance,
  registerService: register,
  unregisterService: unregister
}

/**
 * Register methods which init each frameworks.
 * @param {string} methodName
 */
function genInit (methodName) {
  methods[methodName] = function (...args) {
    if (methodName === 'registerComponents') {
      checkComponentMethods(args[0])
    }
    for (const name in frameworks) {
      const framework = frameworks[name]
      if (framework && framework[methodName]) {
        framework[methodName](...args)
      }
    }
  }
}

function checkComponentMethods (components) {
  if (Array.isArray(components)) {
    components.forEach((name) => {
      if (name && name.type && name.methods) {
        registerElement(name.type, name.methods)
      }
    })
  }
}

/**
 * Register methods which will be called for each instance.
 * @param {string} methodName
 */
function genInstance (methodName) {
  methods[methodName] = function (...args) {
    const id = args[0]
    const info = instanceMap[id]
    if (info && frameworks[info.framework]) {
      const result = frameworks[info.framework][methodName](...args)

      // Lifecycle methods
      if (methodName === 'refreshInstance') {
        services.forEach(service => {
          const refresh = service.options.refresh
          if (refresh) {
            refresh(id, { info, runtime: runtimeConfig })
          }
        })
      }
      else if (methodName === 'destroyInstance') {
        services.forEach(service => {
          const destroy = service.options.destroy
          if (destroy) {
            destroy(id, { info, runtime: runtimeConfig })
          }
        })
        delete instanceMap[id]
      }

      return result
    }
    return new Error(`invalid instance id "${id}"`)
  }
}

/**
 * Adapt some legacy method(s) which will be called for each instance. These
 * methods should be deprecated and removed later.
 * @param {string} methodName
 * @param {string} nativeMethodName
 */
function adaptInstance (methodName, nativeMethodName) {
  methods[nativeMethodName] = function (...args) {
    const id = args[0]
    const info = instanceMap[id]
    if (info && frameworks[info.framework]) {
      return frameworks[info.framework][methodName](...args)
    }
    return new Error(`invalid instance id "${id}"`)
  }
}

export default function init (config) {
  runtimeConfig = config || {}
  frameworks = runtimeConfig.frameworks || {}
  initTaskHandler()

  // Init each framework by `init` method and `config` which contains three
  // virtual-DOM Class: `Document`, `Element` & `Comment`, and a JS bridge method:
  // `sendTasks(...args)`.
  for (const name in frameworks) {
    const framework = frameworks[name]
    framework.init(config)
  }

  // @todo: The method `registerMethods` will be re-designed or removed later.
  ; ['registerComponents', 'registerModules', 'registerMethods'].forEach(genInit)

  ; ['destroyInstance', 'refreshInstance', 'receiveTasks', 'getRoot'].forEach(genInstance)

  adaptInstance('receiveTasks', 'callJS')

  return methods
}
