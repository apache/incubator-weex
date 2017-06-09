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

import { getViewportInfo } from '../env/viewport'
import addPrefix from 'inline-style-prefixer/static/index'

const noUnitsNumberKeys = [
  'flex',
  'opacity',
  'zIndex',
  'fontWeight',
  'lines'
]

/**
 * remove comments from a cssText.
 */
export function trimComment (cssText: string): string {
  return cssText.replace(/(?:\/\*)[\s\S]*?\*\//g, '')
}

let support: boolean | null = null

export function supportSticky (): boolean {
  if (support !== null) {
    return support
  }
  const element = window.document.createElement('div')
  const elementStyle = element.style
  elementStyle.cssText = 'position:-webkit-sticky;position:sticky;'
  support = elementStyle.position.indexOf('sticky') !== -1
  return support
}

const regPercentage = /^[+-]?\d+(\.\d+)?%$/
export function isPercentage (val: string) {
  return regPercentage.test(val)
}

const regUnitsNum = /^([+-]?\d+(?:\.\d+)?)([p,w]x)?$/ // support units: px, wx.
export function normalizeUnitsNum (val: string): string {
  const match = val.match(regUnitsNum)
  if (!match) { return '' }
  let unit = 'px' // px by default.
  if (match[2]) {
    unit = match[2]
  }
  return parseScale(parseFloat(match[1]), unit)
}

function getUnitScaleMap () {
  const { scale, dpr } = getViewportInfo()
  return {
    px: scale,
    wx: scale * dpr
  }
}

function limitScale (val, limit) {
  limit = limit || 1
  const sign = val === 0 ? 0 : val > 0 ? 1 : -1
  return Math.abs(val) > limit ? val : sign * limit
}

function parseScale (val: number, unit: string): string {
  const unitScaleMap = getUnitScaleMap()
  return limitScale(val * unitScaleMap[unit]) + 'px'
}

export function normalizeString (styleKey: string, styleVal: string): string {
  if (isPercentage(styleVal)) {
    return styleVal
  }

  /**
   * 1. test if is a regular scale css. e.g. `width: 100px;`
   *  this should be a standalone number value with or without unit, otherwise
   *  it shouldn't be changed.
   */
  const unitsNum = normalizeUnitsNum(styleVal)
  if (unitsNum) { return unitsNum }

  /**
   * 2. if a string contains multiple px values, than they should be all normalized.
   *  values should have wx or px units, otherwise they should be left unchanged.
   *  e.g.
   *    transform: translate(10px, 6px, 0)
   *    border: 2px solid red
   */
  const numReg = /([+-]?[\d.]+)([p,w]x)/ig
  if (numReg.test(styleVal)) {
    const unitScaleMap = getUnitScaleMap()
    const val = styleVal.replace(numReg, function (m, $0, $1) {
      const res = parseFloat($0) * unitScaleMap[$1]
      return limitScale(res) + 'px'
    })
    return val
  }

  // otherwise
  return styleVal
}

export function autoPrefix (style: {}): {} {
  const prefixed = addPrefix(style)
  // flex only added WebkitFlex. Should add WebkitBoxFlex also.
  const flex = prefixed.flex
  if (flex) {
    prefixed.WebkitBoxFlex = flex
  }
  return prefixed
}

export function normalizeNumber (styleKey: string, styleVal: number): string {
  const { scale } = getViewportInfo()
  return styleVal * scale + 'px'
}

/**
 * normalize style to adapte to current viewport by multiply current scale.
 * @param  {object} style: should be camelCase.
 */
export function normalizeStyle (style: {}) {
  const res = {}
  for (const key in style) {
    const val = style[key]
    if (noUnitsNumberKeys.indexOf(key) > -1) {
      res[key] = val
      continue
    }
    switch (typeof val) {
      case 'string':
        res[key] = normalizeString(key, val)
        break
      case 'number':
        res[key] = normalizeNumber(key, val)
        break
      default:
        res[key] = val
        break
    }
  }
  return res
}

/**
 * get transformObj
 */
export function getTransformObj (elm: HTMLElement): any {
  let styleObj = {}
  if (!elm) { return styleObj }
  const transformStr = elm.style.webkitTransform || elm.style.transform
  if (transformStr && transformStr.match(/(?: *(?:translate|rotate|scale)[^(]*\([^(]+\))+/i)) {
    styleObj = transformStr.trim().replace(/, +/g, ',').split(' ').reduce(function (pre, str) {
      ['translate', 'scale', 'rotate'].forEach(function (name) {
        if (new RegExp(name, 'i').test(str)) {
          pre[name] = str
        }
      })
      return pre
    }, {})
  }
  return styleObj
}

/**
 * translate a transform string from a transformObj.
 */
export function getTransformStr (obj: {}): string {
  return Object.keys(obj).reduce(function (pre, key) {
    return pre + obj[key] + ' '
  }, '')
}

/**
 * add transform style to element.
 * @param {HTMLElement} elm
 * @param {object} style: transform object, format is like this:
 *   {
 *     translate: 'translate3d(2px, 2px, 2px)',
 *     scale: 'scale(0.2)',
 *     rotate: 'rotate(30deg)'
 *   }
 * @param {boolean} replace: whether to replace all transform properties.
 */
export function addTransform (elm: HTMLElement, style: {}, replace: boolean): void {
  if (!style) { return }
  let styleObj = {}
  if (!replace) {
    styleObj = getTransformObj(elm)
  }
  for (const key in style) {
    const val = style[key]
    if (val) {
      styleObj[key] = val
    }
  }
  const resStr = getTransformStr(styleObj)
  elm.style.webkitTransform = resStr
  elm.style.transform = resStr
}

/**
 * add translate X to the element.
 */
export function addTranslateX (elm: HTMLElement, toAdd: number): void {
  if (!toAdd) { return }
  const styleObj = getTransformObj(elm)
  if (!styleObj.translate) {
    styleObj.translate = 'translate3d(0px, 0px, 0px)'
  }
  styleObj.translate = styleObj.translate.replace(/[+-\d.]+[pw]x/, function ($0) {
    return (parseFloat($0) + toAdd) + 'px'
  })
  const resStr = getTransformStr(styleObj)
  elm.style.webkitTransform = resStr
  elm.style.transform = resStr
}

/**
 * copy a transform behaviour from one element to another.
 * key could be: 'translate' | 'scale' | 'rotate'
 */
export function copyTransform (from: HTMLElement, to: HTMLElement, key: string | void): void {
  let str
  if (!key) {
    str = from.style.webkitTransform || from.style.transform
  }
  else {
    const fromObj = getTransformObj(from)
    if (!fromObj[key]) { return }
    const toObj = getTransformObj(to)
    toObj[key] = fromObj[key]
    str = getTransformStr(toObj)
  }
  to.style.webkitTransform = str
  to.style.transform = str
}
