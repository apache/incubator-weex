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
 * Get a unique id.
 */
let nextNodeRef = 1
export function uniqueId () {
  return (nextNodeRef++).toString()
}

export function typof (v) {
  const s = Object.prototype.toString.call(v)
  return s.substring(8, s.length - 1)
}

export function isPlainObject (value) {
  return Object.prototype.toString.call(value) === '[object Object]'
}

export function hasOwn (object, key) {
  return isPlainObject(object) && Object.prototype.hasOwnProperty.call(object, key)
}

export function bufferToBase64 (buffer) {
  if (typeof btoa !== 'function') {
    return ''
  }
  const string = Array.prototype.map.call(
    new Uint8Array(buffer),
    code => String.fromCharCode(code)
  ).join('')
  return btoa(string) // eslint-disable-line no-undef
}

export function base64ToBuffer (base64) {
  if (typeof atob !== 'function') {
    return new ArrayBuffer(0)
  }
  const string = atob(base64) // eslint-disable-line no-undef
  const array = new Uint8Array(string.length)
  Array.prototype.forEach.call(string, (ch, i) => {
    array[i] = ch.charCodeAt(0)
  })
  return array.buffer
}

/**
 * Detect if the param is falsy or empty
 * @param {any} any
 */
export function isEmpty (any) {
  if (!any || typeof any !== 'object') {
    return true
  }

  for (const key in any) {
    if (Object.prototype.hasOwnProperty.call(any, key)) {
      return false
    }
  }
  return true
}

const LEVELS = ['off', 'error', 'warn', 'info', 'log', 'debug']
let levelMap = {}

/**
 * Generate map for which types of message will be sent in a certain message level
 * as the order of LEVELS.
 */
export function generateLevelMap () {
  LEVELS.forEach(level => {
    const levelIndex = LEVELS.indexOf(level)
    levelMap[level] = {}
    LEVELS.forEach(type => {
      const typeIndex = LEVELS.indexOf(type)
      if (typeIndex <= levelIndex) {
        levelMap[level][type] = true
      }
    })
  })
}

/**
 * Reset level map to empty object,
 * then the log level can be toggled at runtime.
 */
export function resetLevelMap () {
  levelMap = {}
}

/**
 * Check if a certain type of message will be sent in current log level of env.
 * @param  {string} type
 * @return {boolean}
 */
export function checkLevel (type) {
  const logLevel = (global.WXEnvironment && global.WXEnvironment.logLevel) || 'log'
  return levelMap[logLevel] && levelMap[logLevel][type]
}

/**
 * Print debug log regardless of level check.
 *
 * This methods can avoid "double level check" compared with using wrapped console.debug
 */
export function debugLog (text) {
  global.nativeLog('wxInteractionAnalyzer: [jsfm]' + text, '__DEBUG')
}
