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
/**
 * @fileOverview
 * instance controls from native
 *
 * - init bundle
 *
 * corresponded with the API of instance manager (framework.js)
 */

import Vm from '../../vm/index'
import { removeWeexPrefix } from '../../util/index'
import {
  defineFn,
  bootstrap,
  register
} from '../bundle/index'
import { updateActions } from './misc'

/**
 * Init an app by run code witgh data
 * @param  {object} app
 * @param  {string} code
 * @param  {object} data
 */
export function init (app, code, data, services) {
  console.debug('[JS Framework] Intialize an instance with:\n', data)
  let result

  // prepare app env methods
  const bundleDefine = (...args) => defineFn(app, ...args)
  const bundleBootstrap = (name, config, _data) => {
    result = bootstrap(app, name, config, _data || data)
    updateActions(app)
    app.doc.listener.createFinish()
    console.debug(`[JS Framework] After intialized an instance(${app.id})`)
  }
  const bundleVm = Vm
  /* istanbul ignore next */
  const bundleRegister = (...args) => register(app, ...args)
  /* istanbul ignore next */
  const bundleRender = (name, _data) => {
    result = bootstrap(app, name, {}, _data)
  }
  /* istanbul ignore next */
  const bundleRequire = name => _data => {
    result = bootstrap(app, name, {}, _data)
  }
  const bundleDocument = app.doc
  /* istanbul ignore next */
  const bundleRequireModule = name => app.requireModule(removeWeexPrefix(name))

  const weexGlobalObject = {
    config: app.options,
    define: bundleDefine,
    bootstrap: bundleBootstrap,
    requireModule: bundleRequireModule,
    document: bundleDocument,
    Vm: bundleVm
  }

  Object.freeze(weexGlobalObject)

  // prepare code
  let functionBody
  /* istanbul ignore if */
  if (typeof code === 'function') {
    // `function () {...}` -> `{...}`
    // not very strict
    functionBody = code.toString().substr(12)
  }
  /* istanbul ignore next */
  else if (code) {
    functionBody = code.toString()
  }
  // wrap IFFE and use strict mode
  functionBody = `(function(global){\n\n"use strict";\n\n ${functionBody} \n\n})(Object.create(this))`

  // run code and get result
  const { WXEnvironment } = global
  const timerAPIs = {}

  /* istanbul ignore if */
  if (WXEnvironment && WXEnvironment.platform !== 'Web') {
    // timer APIs polyfill in native
    const timer = app.requireModule('timer')
    Object.assign(timerAPIs, {
      setTimeout: (...args) => {
        const handler = function () {
          args[0](...args.slice(2))
        }

        let timerId, callbackId

        if (global.setTimeoutWeex && (typeof global.setTimeoutWeex === 'function')) {
          callbackId = app.doc.taskCenter.normalize(handler)
          timerId = global.setTimeoutWeex(app.id, callbackId, args[1])
          return timerId
        }
        else {
          timer.setTimeout(handler, args[1])
          return app.doc.taskCenter.callbackManager.lastCallbackId.toString()
        }
      },
      setInterval: (...args) => {
        const handler = function () {
          args[0](...args.slice(2))
        }

        let timerId, callbackId

        if (global.setIntervalWeex && (typeof global.setIntervalWeex === 'function')) {
          callbackId = app.doc.taskCenter.normalize(handler)
          timerId = global.setIntervalWeex(app.id, callbackId, args[1])
          return timerId
        }
        else {
          timer.setInterval(handler, args[1])
          return app.doc.taskCenter.callbackManager.lastCallbackId.toString()
        }
      },
      clearTimeout: (n) => {
        if (global.clearTimeoutWeex && (typeof global.clearTimeoutWeex === 'function')) {
          global.clearTimeoutWeex(n)
        }
        else {
          timer.clearTimeout(n)
        }
      },
      clearInterval: (n) => {
        if (global.clearIntervalWeex && (typeof global.clearIntervalWeex === 'function')) {
          global.clearIntervalWeex(n)
        }
        else {
          timer.clearInterval(n)
        }
      }
    })
  }
  // run code and get result
  const globalObjects = Object.assign({
    define: bundleDefine,
    require: bundleRequire,
    bootstrap: bundleBootstrap,
    register: bundleRegister,
    render: bundleRender,
    __weex_define__: bundleDefine, // alias for define
    __weex_bootstrap__: bundleBootstrap, // alias for bootstrap
    __weex_document__: bundleDocument,
    __weex_require__: bundleRequireModule,
    __weex_viewmodel__: bundleVm,
    weex: weexGlobalObject
  }, timerAPIs, services)
  if (!callFunctionNative(globalObjects, functionBody)) {
    // If failed to compile functionBody on native side,
    // fallback to callFunction.
    callFunction(globalObjects, functionBody)
  }

  return result
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
 * Call a new function generated on the V8 native side.
 * @param  {object} globalObjects
 * @param  {string} body
 * @return {boolean} return true if no error occurred.
 */
function callFunctionNative (globalObjects, body) {
  if (typeof compileAndRunBundle !== 'function') {
    return false
  }

  let fn = void 0
  let isNativeCompileOk = false
  let script = '(function ('
  const globalKeys = []
  const globalValues = []
  for (const key in globalObjects) {
    globalKeys.push(key)
    globalValues.push(globalObjects[key])
  }
  for (let i = 0; i < globalKeys.length - 1; ++i) {
    script += globalKeys[i]
    script += ','
  }
  script += globalKeys[globalKeys.length - 1]
  script += ') {'
  script += body
  script += '} )'

  try {
    const weex = globalObjects.weex || {}
    const config = weex.config || {}
    fn = compileAndRunBundle(script,
                             config.bundleUrl,
                             config.bundleDigest,
                             config.codeCachePath)
    if (fn && typeof fn === 'function') {
      fn(...globalValues)
      isNativeCompileOk = true
    }
  }
  catch (e) {
    console.error(e)
  }

  return isNativeCompileOk
}
