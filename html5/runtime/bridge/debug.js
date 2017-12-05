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

import { getTaskCenter } from '../vdom/operation'
import { isRegisteredModule } from '../api/module'

export function track (id, type, value) {
  const taskCenter = getTaskCenter(id)
  if (!taskCenter || typeof taskCenter.send !== 'function') {
    console.error(`[JS Framework] Failed to create tracker!`)
    return
  }
  if (!type || !value) {
    console.warn(`[JS Framework] Invalid track type (${type}) or value (${value})`)
    return
  }
  const label = `jsfm.${type}.${value}`
  try {
    if (isRegisteredModule('userTrack', 'addPerfPoint')) {
      const message = Object.create(null)
      message[label] = '4'
      taskCenter.send('module', {
        module: 'userTrack',
        method: 'addPerfPoint'
      }, [message])
    }
  }
  catch (err) {
    console.error(`[JS Framework] Failed to trace "${label}"!`)
  }
}

export function error (...messages) {
  if (typeof console.error === 'function') {
    console.error(`[JS Framework] `, ...messages)
  }
}

export function handleException (err) {
  if (typeof console.error === 'function') {
    try {
      console.error(err.toString())
    }
    catch (e) {}
  }
  if (process.env.NODE_ENV !== 'production') {
    throw err
  }
}
