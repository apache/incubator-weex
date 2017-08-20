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
// @flow
import {
  isArray
} from './type'
/**
 * Mix properties into target object.
 * the rightest object's value has the highest priority.
 */
export function extend (to: {}, ...args: Array<{}>): {} {
  if (!args || args.length <= 0) {
    return to
  }
  args.forEach(from => {
    if (typeof from !== 'object') {
      return
    }
    for (const key in from) {
      to[key] = from[key]
    }
  })
  return to
}
/**
 * Mix truthy or '' property values into target object.
 * mostly for merging styles. (that's why '' is falsy but still should be counted in.)
 * the rightest object's value has the highest priority.
 */
export function extendTruthy (to: {}, ...args: Array<{}>): {} {
  if (!args || args.length <= 0) {
    return to
  }
  args.forEach(from => {
    if (typeof from !== 'object') {
      return
    }
    let i
    for (const key in from) {
      if (((i = from[key]) || i === '' || i === 0) && i !== 'undefined') {
        to[key] = i
      }
    }
  })
  return to
}
/**
 * Mix specified properties into target object.
 */
export function extendKeys (to: {}, from: {} = {}, keys: Array<string>): {} {
  (keys || []).forEach(key => {
    from && (to[key] = from[key])
  })
  return to
}
/**
 * Extract specified properties from src to target object.
 */
export function extractKeys (to: {}, from: {} = {}, keys: Array<string>) {
  if (!from) {
    return to
  }
  (keys || []).forEach(key => {
    from && (to[key] = from[key])
    from && (delete from[key])
  })
  return to
}
/**
 * Simple bind, faster than native
 *
 * @param {Function} fn
 * @param {Object} ctx
 * @return {Function}
 */
export function bind (fn: Function, ctx: mixed) {
  return function (a: mixed) {
    const l = arguments.length
    return l ? l > 1 ? fn.apply(ctx, arguments) : fn.call(ctx, a) : fn.call(ctx)
  }
}
/**
 * Only call the func the last time before it's not that frequently called.
 */
export function debounce (func: Function, wait: number) {
  let timerId
  function later () {
    timerId = null
    func.apply(null)
  }
  return function () {
    clearTimeout(timerId)
    timerId = setTimeout(later, wait)
  }
}
/**
 * Only call the func the first time before a series frequently function calls happen.
 */
export function depress (func: Function, wait: number) {
  let timerId

  function later () {
    timerId = null
  }
  return function () {
    if (!timerId) {
      func.apply()
    }
    clearTimeout(timerId)
    timerId = setTimeout(later, wait)
  }
}
/**
 * Only call the func every time after a wait milliseconds if it's too frequently called.
 */
export function throttle (func: Function, wait: number, callLastTime: boolean) {
  let last = 0
  let lastTimer = null
  const lastTimeDuration = wait + (wait > 25 ? wait : 25) // plus half wait time.
  return function (...args: Array<mixed>) {
    const context = this
    const time = new Date().getTime()
    if (time - last > wait) {
      if (callLastTime) {
        lastTimer && clearTimeout(lastTimer)
        lastTimer = setTimeout(function () {
          lastTimer = null
          func.apply(context, args)
        }, lastTimeDuration)
      }
      func.apply(context, args)
      last = time
    }
  }
}
// direction: 'l' | 'r', default is 'r'
// num: how many times to loop, should be a positive integer
export function loopArray (arr: any, num: number, direction: string) {
  if (!isArray(arr)) {
    return
  }
  let isLeft = (direction + '').toLowerCase() === 'l'
  const len = arr.length
  num = num % len
  if (num < 0) {
    num = -num
    isLeft = !isLeft
  }
  if (num === 0) {
    return arr
  }
  let lp, rp
  if (isLeft) {
    lp = arr.slice(0, num)
    rp = arr.slice(num)
  }
  else {
    lp = arr.slice(0, len - num)
    rp = arr.slice(len - num)
  }
  return rp.concat(lp)
}
/**
 * Create a cached version of a pure function.
 */
export function cached (fn: any) {
  const cache = Object.create(null)
  return function cachedFn (str: string) {
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
export function camelizeKeys (obj: any) {
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
  return str.replace(hyphenateRE, '$1-$2').replace(hyphenateRE, '$1-$2').toLowerCase()
})
export function hyphenateKeys (obj: any) {
  const res = {}
  for (const key in obj) {
    res[hyphenate(key)] = obj[key]
  }
  return res
}
const vendorsReg = /webkit-|moz-|o-|ms-/
export function hyphenateStyleKeys (obj: any) {
  const res = {}
  for (const key in obj) {
    const hk = hyphenate(key).replace(vendorsReg, function ($0) {
      return '-' + $0
    })
    res[hk] = obj[key]
  }
  return res
}
export function camelToKebab (name: string) {
  if (!name) {
    return ''
  }
  return name.replace(/([A-Z])/g, function (g, g1) {
    return `-${g1.toLowerCase()}`
  })
}
export function appendCss (css: string, cssId: string, replace: boolean) {
  let style: any = document.getElementById(cssId)
  if (style && replace) {
    style.parentNode.removeChild(style)
    style = null
  }
  if (!style) {
    style = document.createElement('style')
    style.type = 'text/css'
    cssId && (style.id = cssId)
    document.getElementsByTagName('head')[0].appendChild(style)
  }
  style.appendChild(document.createTextNode(css))
}
export function nextFrame (callback: any) {
  const runner = window.requestAnimationFrame || window.webkitRequestAnimationFrame || (cb => setTimeout(cb, 16))
  runner(callback)
}
export function toCSSText (object: any) {
  if (!object) {
    return
  }
  const obj = hyphenateStyleKeys(object)
  let cssText = ''
  for (const key in obj) {
    cssText += `${key}:${obj[key]};`
  }
  return cssText
}
