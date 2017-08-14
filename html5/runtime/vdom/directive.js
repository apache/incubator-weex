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

import { typof } from '../utils'

// match the binding delimiter
const delimiterRE = /\[\[((?:.|\n)+?)\]\]/g

export function generateBinding (text) {
  if (typof(text) === 'String') {
    return { '@binding': text }
  }
  return text
}

export function parseString (string) {
  const tokens = []
  let lastIndex = delimiterRE.lastIndex = 0
  let match, index
  while ((match = delimiterRE.exec(string))) {
    index = match.index
    if (index > lastIndex) {
      tokens.push(string.slice(lastIndex, index))
    }
    const binding = generateBinding(match[1].trim())
    tokens.push(binding)
    lastIndex = index + match[0].length
  }
  if (lastIndex < string.length) {
    tokens.push(string.slice(lastIndex))
  }
  if (tokens.length === 1) {
    return tokens[0]
  }
  return tokens
}

export function filterDirective (value) {
  if (typof(value) === 'String' && delimiterRE.test(value)) {
    return parseString(value)
  }
  if (typof(value) === 'Object') {
    const realData = {}
    for (const key in value) {
      realData[key] = filterDirective(value[key])
    }
    return realData
  }
  if (typof(value) === 'Array') {
    return value.map(filterDirective)
  }
  return value
}
