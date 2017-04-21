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
'use strict'

const NOT_PX_NUMBER_PROPERTIES = ['flex', 'opacity', 'zIndex', 'fontWeight']

export function filterStyles (styles) {
  for (const key in styles) {
    const value = styles[key]
    const parser = this.getFilters(key)[typeof value]
    if (typeof parser === 'function') {
      styles[key] = parser(value)
    }
  }
}

export function getFilters (key) {
  if (NOT_PX_NUMBER_PROPERTIES.indexOf(key) !== -1) {
    return {}
  }
  return {
    number: function (val) {
      return val + 'px'
    },
    string: function (val) {
      // string of a number suffixed with a 'px' or 'wx' unit. original RegExp is /^-?\d*\.?\d+(?:px)?$/
      const match = val.match(/^([+-]?\d.*)+([p,w]x)$/)
      if (match && match.length === 3) {
        if (match[2] === 'px') {
          return parseFloat(match[1]) + 'px'
        }
        else if (match[2] === 'wx') {
          return parseFloat(match[1]) * global.WXEnvironment.devicePixelRatio + 'px'
        }
      }
      if (key.match(/transform/) && val.match(/translate/)) {
        let ret = val.replace(/\d*\.?\d+px/g, function (match) {
          return parseInt(parseFloat(match)) + 'px'
        })
        if (ret.match(/wx/)) {
          ret = ret.replace(/\d*\.?\d+wx/g, function (match) {
            return parseInt(parseFloat(match)) * global.WXEnvironment.devicePixelRatio + 'px'
          })
        }
        return ret
      }
      if (key.match(/^border$/) && val.match(/^\d+(?:[w,p]x)?\s+/)) {
        val = val.replace(/^(\d+(?:[w,p]x)?)/, function ($0, $1) {
          const v = parseFloat($1) * (val.match(/^\d+(?:wx)+/) ? global.WXEnvironment.devicePixelRatio : 1)
          return v + 'px'
        })
      }
      return val
    }
  }
}
