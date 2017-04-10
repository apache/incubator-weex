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
export * from './func'
export * from './event'
export * from './component'
export * from './lazyload'
export * from './style'
export * from './type'

/**
 * Create a cached version of a pure function.
 */
export function cached (fn) {
  const cache = Object.create(null)
  return function cachedFn (str) {
    const hit = cache[str]
    return hit || (cache[str] = fn(str))
  }
}

/**
 * Camelize a hyphen-delmited string.
 */
const camelizeRE = /-(\w)/g
export const camelize = cached(str => {
  return str.replace(camelizeRE, (_, c) => c.toUpperCase())
})

export function camelizeKeys (obj) {
  const res = {}
  for (const key in obj) {
    res[camelize(key)] = obj[key]
  }
  return res
}

/**
 * Capitalize a string.
 */
export const capitalize = cached(str => {
  return str.charAt(0).toUpperCase() + str.slice(1)
})

/**
 * Hyphenate a camelCase string.
 */
const hyphenateRE = /([^-])([A-Z])/g
export const hyphenate = cached(str => {
  return str
    .replace(hyphenateRE, '$1-$2')
    .replace(hyphenateRE, '$1-$2')
    .toLowerCase()
})

export function hyphenateKeys (obj) {
  const res = {}
  for (const key in obj) {
    res[hyphenate(key)] = obj[key]
  }
  return res
}

// const vendorsReg = /webkit-|moz-|o-|ms-/
// export function hyphenateStyleKeys (obj) {
//   const res = {}
//   for (const key in obj) {
//     const hk = hyphenate(key).replace(vendorsReg, function ($0) {
//       return '-' + $0
//     })
//     res[hk] = obj[key]
//   }
//   return res
// }

export function camelToKebab (name) {
  if (!name) { return '' }
  return name.replace(/([A-Z])/g, function (g, g1) {
    return `-${g1.toLowerCase()}`
  })
}

export function appendStyle (css, styleId, replace) {
  let style = document.getElementById(styleId)
  if (style && replace) {
    style.parentNode.removeChild(style)
    style = null
  }
  if (!style) {
    style = document.createElement('style')
    style.type = 'text/css'
    styleId && (style.id = styleId)
    document.getElementsByTagName('head')[0].appendChild(style)
  }
  style.appendChild(document.createTextNode(css))
}

export function nextFrame (callback) {
  const runner = window.requestAnimationFrame
    || window.webkitRequestAnimationFrame
    || (cb => setTimeout(cb, 16))
  runner(callback)
}

export function toCSSText (object) {
  let cssText = ''
  if (object) {
    for (const key in object) {
      cssText += `${hyphenate(key)}:${object[key]};`
    }
  }
  return cssText
}
