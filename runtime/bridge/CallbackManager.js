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

import { decodePrimitive } from './normalize'

function getHookKey (componentId, type, hookName) {
  return `${type}@${hookName}#${componentId}`
}

/**
 * For general callback management of a certain Weex instance.
 * Because function can not passed into native, so we create callback
 * callback id for each function and pass the callback id into native
 * in fact. And when a callback called from native, we can find the real
 * callback through the callback id we have passed before.
 */
export default class CallbackManager {
  constructor (instanceId) {
    this.instanceId = String(instanceId)
    this.lastCallbackId = 0
    this.callbacks = {}
    this.hooks = {}
  }
  add (callback) {
    this.lastCallbackId++
    this.callbacks[this.lastCallbackId] = callback
    return this.lastCallbackId
  }
  remove (callbackId) {
    const callback = this.callbacks[callbackId]
    delete this.callbacks[callbackId]
    return callback
  }
  registerHook (componentId, type, hookName, hookFunction) {
    // TODO: validate arguments
    const key = getHookKey(componentId, type, hookName)
    if (this.hooks[key]) {
      console.warn(`[JS Framework] Override an existing component hook "${key}".`)
    }
    this.hooks[key] = hookFunction
  }
  triggerHook (componentId, type, hookName, options = {}) {
    // TODO: validate arguments
    const key = getHookKey(componentId, type, hookName)
    const hookFunction = this.hooks[key]
    if (typeof hookFunction !== 'function') {
      console.error(`[JS Framework] Invalid hook function type (${typeof hookFunction}) on "${key}".`)
      return null
    }
    let result = null
    try {
      result = hookFunction.apply(null, options.args || [])
    }
    catch (e) {
      console.error(`[JS Framework] Failed to execute the hook function on "${key}".`)
    }
    return result
  }
  consume (callbackId, data, ifKeepAlive) {
    const callback = this.callbacks[callbackId]
    if (typeof ifKeepAlive === 'undefined' || ifKeepAlive === false) {
      delete this.callbacks[callbackId]
    }
    if (typeof callback === 'function') {
      return callback(decodePrimitive(data))
    }
    return new Error(`invalid callback id "${callbackId}"`)
  }
  close () {
    this.callbacks = {}
    this.hooks = {}
  }
}
