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
const config = {}

const instanceMap = {}

export function init (cfg) {
  config.Document = cfg.Document
  config.Element = cfg.Element
  config.Comment = cfg.Comment
  config.sendTasks = cfg.sendTasks
}

export function registerComponents (components) {}

export function registerModules (modules) {}

export function registerMethods (apis) {}

export function prepareInstance (id, options, data) {}

export function createInstance (id, code, options, data, serviceObjects) {
  const document = new config.Document(id, options.bundleUrl)
  const callbacks = {}

  let lastCallbackId = 0
  document.addCallback = func => {
    lastCallbackId++
    callbacks[lastCallbackId] = func
    return lastCallbackId
  }
  document.handleCallback = (funcId, data, ifLast) => {
    const callback = callbacks[funcId]
    if (ifLast) {
      delete callbacks[funcId]
    }
    return callback(data)
  }
  instanceMap[id] = document

  const globalObjects = Object.assign({
    Document: config.Document,
    Element: config.Element,
    Comment: config.Comment,
    sendTasks: config.sendTasks,
    id, options, data, document
  }, serviceObjects)

  const globalKeys = []
  const globalValues = []
  for (const key in globalObjects) {
    globalKeys.push(key)
    globalValues.push(globalObjects[key])
  }
  globalKeys.push(code)

  const result = new Function(...globalKeys)
  result(...globalValues)

  return { document }
}

export function refreshInstance (id, data) {}

export function destroyInstance (id) {
  delete instanceMap[id]
}

export function getRoot (id) {
  return instanceMap[id].body.toJSON()
}

export function receiveTasks (id, tasks) {
  const jsHandlers = {
    fireEvent: (id, ref, type, data, domChanges) => {
      const document = instanceMap[id]
      const el = document.getRef(ref)
      return document.fireEvent(el, type, data, domChanges)
    },

    callback: (id, funcId, data, ifLast) => {
      const document = instanceMap[id]
      return document.handleCallback(funcId, data, ifLast)
    }
  }

  const document = instanceMap[id]
  if (document && Array.isArray(tasks)) {
    const results = []
    tasks.forEach((task) => {
      const handler = jsHandlers[task.method]
      const args = [...task.args]
      if (typeof handler === 'function') {
        args.unshift(id)
        results.push(handler(...args))
      }
    })
    return results
  }
}
