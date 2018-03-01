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
