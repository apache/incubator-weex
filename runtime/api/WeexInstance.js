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

import Document from '../vdom/Document'
import { isRegisteredModule, getModuleDescription } from './module'
import { isRegisteredComponent } from './component'

const moduleProxies = {}

function setId (weex, id) {
  Object.defineProperty(weex, '[[CurrentInstanceId]]', { value: id })
}

function getId (weex) {
  return weex['[[CurrentInstanceId]]']
}

function moduleGetter (module, method, taskCenter) {
  return (...args) => taskCenter.send('module', { module, method }, args)
}

export default class WeexInstance {
  constructor (id, config) {
    setId(this, id)
    this.config = config || {}
    this.document = new Document(id, this.config.bundleUrl)
    this.requireModule = this.requireModule.bind(this)
    this.isRegisteredModule = isRegisteredModule
    this.isRegisteredComponent = isRegisteredComponent
  }

  requireModule (moduleName) {
    const id = getId(this)
    if (!(id && this.document && this.document.taskCenter)) {
      console.error(`[JS Framework] invalid instance id "${id}"`)
      return
    }

    // warn for unknown module
    if (!isRegisteredModule(moduleName)) {
      console.warn(`[JS Framework] using unregistered weex module "${moduleName}"`)
      return
    }

    // create new module proxy
    if (!moduleProxies[moduleName]) {
      const moduleDefine = getModuleDescription(moduleName)
      const taskCenter = this.document.taskCenter

      // create registered module apis
      const moduleApis = {}
      for (const methodName in moduleDefine) {
        Object.defineProperty(moduleApis, methodName, {
          enumerable: true,
          configurable: true,
          get: () => moduleGetter(moduleName, methodName, taskCenter),
          set (fn) {
            if (typeof fn === 'function') {
              return taskCenter.send('module', {
                module: moduleName,
                method: methodName
              }, [fn])
            }
          }
        })
      }

      // create module Proxy
      if (typeof Proxy === 'function') {
        moduleProxies[moduleName] = new Proxy(moduleApis, {
          get (target, methodName) {
            if (methodName in target) {
              return target[methodName]
            }
            console.warn(`[JS Framework] using unregistered method "${moduleName}.${methodName}"`)
            return moduleGetter(moduleName, methodName, taskCenter)
          }
        })
      }
      else {
        moduleProxies[moduleName] = moduleApis
      }
    }

    return moduleProxies[moduleName]
  }

  supports (condition) {
    if (typeof condition !== 'string') return null

    const res = condition.match(/^@(\w+)\/(\w+)(\.(\w+))?$/i)
    if (res) {
      const type = res[1]
      const name = res[2]
      const method = res[4]
      switch (type) {
        case 'module': return isRegisteredModule(name, method)
        case 'component': return isRegisteredComponent(name)
      }
    }

    return null
  }

  // registerStyleSheet (styles) {
  //   if (this.document) {
  //     this.document.registerStyleSheet(styles)
  //   }
  // }
}
