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

import { hasOwn, isPlainObject } from '../shared/utils'

const weexModules = {}

/**
 * Register native modules information.
 * @param {object} newModules
 */
export function registerModules (newModules = {}) {
  for (const name in newModules) {
    if (!hasOwn(weexModules, name)) {
      weexModules[name] = {}
    }
    if (Array.isArray(newModules[name])) {
      newModules[name].forEach(method => {
        if (typeof method === 'string') {
          weexModules[name][method] = true
        }
        else if (isPlainObject(method) && typeof method.name === 'string') {
          weexModules[name][method.name] = method.args || []
        }
      })
    }
  }
}

/**
 * Check whether the module or the method has been registered.
 * @param {String} module name
 * @param {String} method name (optional)
 */
export function isRegisteredModule (name, method) {
  if (typeof method === 'string') {
    return !!(weexModules[name] && weexModules[name][method])
  }
  return !!weexModules[name]
}

export function getModuleDescription (name) {
  return weexModules[name]
}
