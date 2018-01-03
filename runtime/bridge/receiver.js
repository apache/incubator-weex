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

import { getDoc } from '../vdom/operation'

function fireEvent (document, nodeId, type, event, domChanges, params) {
  const el = document.getRef(nodeId)
  if (el) {
    return document.fireEvent(el, type, event, domChanges, params)
  }
  return new Error(`invalid element reference "${nodeId}"`)
}

function callback (document, callbackId, data, ifKeepAlive) {
  return document.taskCenter.callback(callbackId, data, ifKeepAlive)
}

function componentHook (document, componentId, type, hook, options) {
  if (!document || !document.taskCenter) {
    console.error(`[JS Framework] Can't find "document" or "taskCenter".`)
    return null
  }
  let result = null
  try {
    result = document.taskCenter.triggerHook(componentId, type, hook, options)
  }
  catch (e) {
    console.error(`[JS Framework] Failed to trigger the "${type}@${hook}" hook on ${componentId}.`)
  }
  return result
}

/**
 * Accept calls from native (event or callback).
 *
 * @param  {string} id
 * @param  {array} tasks list with `method` and `args`
 */
export function receiveTasks (id, tasks) {
  const document = getDoc(id)
  if (!document) {
    return new Error(`[JS Framework] Failed to receiveTasks, `
      + `instance (${id}) is not available.`)
  }
  if (Array.isArray(tasks)) {
    return tasks.map(task => {
      switch (task.method) {
        case 'callback': return callback(document, ...task.args)
        case 'fireEventSync':
        case 'fireEvent': return fireEvent(document, ...task.args)
        case 'componentHook': return componentHook(document, ...task.args)
      }
    })
  }
}
