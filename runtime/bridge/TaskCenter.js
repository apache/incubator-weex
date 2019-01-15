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

import CallbackManager from './CallbackManager'
import Element from '../vdom/Element'
import { typof, checkLevel, debugLog } from '../shared/utils'
import { normalizePrimitive } from './normalize'

let fallback = function () {}

// The API of TaskCenter would be re-design.
export class TaskCenter {
  constructor (id, sendTasks) {
    Object.defineProperty(this, 'instanceId', {
      enumerable: true,
      value: String(id)
    })
    Object.defineProperty(this, 'callbackManager', {
      enumerable: true,
      value: new CallbackManager(id)
    })
    fallback = sendTasks || function () {}
  }

  callback (callbackId, data, ifKeepAlive) {
    return this.callbackManager.consume(callbackId, data, ifKeepAlive)
  }

  registerHook (...args) {
    return this.callbackManager.registerHook(...args)
  }

  triggerHook (...args) {
    return this.callbackManager.triggerHook(...args)
  }

  updateData (componentId, newData, callback) {
    this.send('module', {
      module: 'dom',
      method: 'updateComponentData'
    }, [componentId, newData, callback])
  }

  destroyCallback () {
    return this.callbackManager.close()
  }

  /**
   * Normalize a value. Specially, if the value is a function, then generate a function id
   * and save it to `CallbackManager`, at last return the function id.
   * @param  {any}        v
   * @return {primitive}
   */
  normalize (v, deep = false) {
    const type = typof(v)
    if (v && v instanceof Element) {
      return v.ref
    }
    if (v && v._isVue && v.$el instanceof Element) {
      return v.$el.ref
    }
    if (type === 'Function') {
      return this.callbackManager.add(v).toString()
    }
    if (deep) {
      if (type === 'Object') {
        const object = {}
        for (const key in v) {
          object[key] = this.normalize(v[key], true)
        }
        return object
      }
      if (type === 'Array') {
        return v.map(item => this.normalize(item, true))
      }
    }
    if (v && v.ref && v['[[VirtualElement]]']) {
      return v.ref
    }
    return normalizePrimitive(v)
  }

  send (type, params, args, options) {
    const { action, component, ref, module, method } = params

    // normalize args and options
    args = args.map(arg => this.normalize(arg))
    if (typof(options) === 'Object') {
      options = this.normalize(options, true)
    }

    switch (type) {
      case 'dom': {
        if (checkLevel('debug')) {
          debugLog(`[task](${this.instanceId},${this.type},${action}) ${JSON.stringify(args)}`)
        }
        return this[action](this.instanceId, args)
      }
      case 'component':
        return this.componentHandler(this.instanceId, ref, method, args, Object.assign({ component }, options))
      default: {
        if (checkLevel('debug')) {
          debugLog(`[task](${this.instanceId},${module},${method}) ${JSON.stringify(args)}`)
        }
        return this.moduleHandler(this.instanceId, module, method, args, options)
      }
    }
  }

  callDOM (action, args) {
    return this[action](this.instanceId, args)
  }

  callComponent (ref, method, args, options) {
    return this.componentHandler(this.instanceId, ref, method, args, options)
  }

  callModule (module, method, args, options) {
    return this.moduleHandler(this.instanceId, module, method, args, options)
  }
}

export function init () {
  const DOM_METHODS = {
    createFinish: global.callCreateFinish,
    updateFinish: global.callUpdateFinish,
    refreshFinish: global.callRefreshFinish,

    createBody: global.callCreateBody,

    addElement: global.callAddElement,
    removeElement: global.callRemoveElement,
    moveElement: global.callMoveElement,
    updateAttrs: global.callUpdateAttrs,
    updateStyle: global.callUpdateStyle,

    addEvent: global.callAddEvent,
    removeEvent: global.callRemoveEvent,
    __updateComponentData: global.__updateComponentData
  }
  const proto = TaskCenter.prototype

  for (const name in DOM_METHODS) {
    const method = DOM_METHODS[name]
    proto[name] = method ?
      (id, args) => method(id, ...args) :
      (id, args) => fallback(id, [{ module: 'dom', method: name, args }], '-1')
  }

  proto.componentHandler = global.callNativeComponent ||
    ((id, ref, method, args, options) =>
      fallback(id, [{ component: options.component, ref, method, args }]))

  proto.moduleHandler = global.callNativeModule ||
    ((id, module, method, args) =>
      fallback(id, [{ module, method, args }]))
}
