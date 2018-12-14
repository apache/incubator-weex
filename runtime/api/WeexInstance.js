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
import { getTaskCenter } from '../vdom/operation'

function setId (weex, id) {
  Object.defineProperty(weex, '[[CurrentInstanceId]]', { value: id })
}

function getId (weex) {
  return weex['[[CurrentInstanceId]]']
}

function moduleGetter (id, module, method) {
  const taskCenter = getTaskCenter(id)
  if (!taskCenter || typeof taskCenter.send !== 'function') {
    console.error(`[JS Framework] Failed to find taskCenter (${id}).`)
    return null
  }
  return (...args) => taskCenter.send('module', { module, method }, args)
}

function moduleSetter (id, module, method, fn) {
  const taskCenter = getTaskCenter(id)
  if (!taskCenter || typeof taskCenter.send !== 'function') {
    console.error(`[JS Framework] Failed to find taskCenter (${id}).`)
    return null
  }
  if (typeof fn !== 'function') {
    console.error(`[JS Framework] ${module}.${method} must be assigned as a function.`)
    return null
  }
  return fn => taskCenter.send('module', { module, method }, [fn])
}

export default class WeexInstance {
  constructor (id, config, data) {
    setId(this, String(id))
    this.config = config || {}
    this._nativeData = data || {}
    this.document = new Document(id, this.config.bundleUrl)
    this.requireModule = this.requireModule.bind(this)
    this.importScript = this.importScript.bind(this)
    this.isRegisteredModule = isRegisteredModule
    this.isRegisteredComponent = isRegisteredComponent
  }

  requireModule (moduleName) {
    const id = getId(this)
    if (!(id && this.document && this.document.taskCenter)) {
      console.error(`[JS Framework] Failed to requireModule("${moduleName}"), `
        + `instance (${id}) doesn't exist anymore.`)
      return
    }

    // warn for unknown module
    if (!isRegisteredModule(moduleName)) {
      console.warn(`[JS Framework] using unregistered weex module "${moduleName}"`)
      return
    }

    // create registered module apis
    const moduleProxy = {}
    const moduleDefine = getModuleDescription(moduleName)
    for (const methodName in moduleDefine) {
      Object.defineProperty(moduleProxy, methodName, {
        enumerable: true,
        configurable: true,
        get: () => moduleGetter(id, moduleName, methodName),
        set: fn => moduleSetter(id, moduleName, methodName, fn)
      })
    }

    return moduleProxy
  }

  supports (condition) {
    if (typeof condition !== 'string') return null

    const res = condition.match(/^@(\w+)\/([\w-]+)(\.(\w+))?$/i)
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

  importScript (src, options = {}) {
    const id = getId(this)
    const taskCenter = getTaskCenter(id)
    return new Promise(function (resolve, reject) {
      if (!taskCenter || typeof taskCenter.send !== 'function') {
        reject(new Error(`[JS Framework] Failed to import script "${src}", `
          + `no taskCenter (${id}) matched.`))
      }
      try {
        taskCenter.send('module', {
          module: 'script',
          method: 'importScript'
        }, [src, options], {
          callback: [
            result => resolve(result),
            error => reject(error)
          ]
        })
      }
      catch (e) {
        reject(e)
      }
    })
  }

  // registerStyleSheet (styles) {
  //   if (this.document) {
  //     this.document.registerStyleSheet(styles)
  //   }
  // }
}
