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
let nativeModules = {}

// for testing

/**
 * for testing
 */
export function getModule (moduleName) {
  return nativeModules[moduleName]
}

/**
 * for testing
 */
export function clearModules () {
  nativeModules = {}
}

// for framework

/**
 * init modules for an app instance
 * the second param determines whether to replace an existed method
 */
export function initModules (modules, ifReplace) {
  for (const moduleName in modules) {
    // init `modules[moduleName][]`
    let methods = nativeModules[moduleName]
    if (!methods) {
      methods = {}
      nativeModules[moduleName] = methods
    }

    // push each non-existed new method
    if (Array.isArray(modules[moduleName])) {
      modules[moduleName].forEach(function (method) {
        if (typeof method === 'string') {
          method = {
            name: method
          }
        }

        if (!methods[method.name] || ifReplace) {
          methods[method.name] = method
        }
      })
    }
  }
}

/**
 * init app methods
 */
export function initMethods (Vm, apis) {
  const p = Vm.prototype

  for (const apiName in apis) {
    if (!p.hasOwnProperty(apiName)) {
      p[apiName] = apis[apiName]
    }
  }
}

/**
 * get a module of methods for an app instance
 */
export function requireModule (app, name) {
  const methods = nativeModules[name]
  const target = {}
  for (const methodName in methods) {
    Object.defineProperty(target, methodName, {
      configurable: true,
      enumerable: true,
      get: function moduleGetter () {
        return (...args) => app.callTasks({
          module: name,
          method: methodName,
          args: args
        })
      },
      set: function moduleSetter (value) {
        if (typeof value === 'function') {
          return app.callTasks({
            module: name,
            method: methodName,
            args: [value]
          })
        }
      }
    })
  }
  return target
}

/**
 * get a custom component options
 */
export function requireCustomComponent (app, name) {
  const { customComponentMap } = app
  return customComponentMap[name]
}

/**
 * register a custom component options
 */
export function registerCustomComponent (app, name, def) {
  const { customComponentMap } = app

  if (customComponentMap[name]) {
    console.error(`[JS Framework] define a component(${name}) that already exists`)
    return
  }

  customComponentMap[name] = def
}
