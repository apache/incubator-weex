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
  return cssText.replace(/(?:\/\*)[\s\S]*\*\//g, '')
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

function parseScale (val: number, unit: string): string {
  const { scale, dpr } = getViewportInfo()
  const unitScaleMap = {
    px: scale,
    wx: scale * dpr
  }
  return val * unitScaleMap[unit] + 'px'
}

export function normalizeString (styleKey: string, styleVal: string): string {
  if (isPercentage(styleVal)) {
    return styleVal
  }

  // 1. test if is a regular scale css. e.g. `width: 100px;`
  const unitsNum = normalizeUnitsNum(styleVal)
  if (unitsNum) { return unitsNum }

  // 2. test if is a translate scale. e.g. `transform: translate2d(1px, 2px);`
  const regTranslateString = /translate[^(]*\([\d ,.pwx]+\)/i // unit support: wx, px.
  if (styleKey.match(/transform/i) && regTranslateString.test(styleVal)) {
    const val = styleVal.replace(regTranslateString, function (translate) {
      const reg = /([+-]?\d+(?:\.\d+)?)([p,w]x)?(?![dD])/g
      return translate.replace(reg, function (m, $1, $2) {
        const unit = $2 || 'px'
        return parseScale($1, unit)
      })
    })
    return val
  }

  // 3. test if is a border style. e.g. `border: 1px solid red;`
  const regBorderKey = /^border(?:-(?:top|bottom|left|right))?$/
  const regBorderVal = /^([+-]?\d+(?:\.\d+)?)([p ,w]x)?\s+/
  if (regBorderKey.test(styleKey) && regBorderVal.test(styleVal)) {
    const reg = /^([+-]?\d+(?:\.\d+)?)([p,w]x)?/
    const val = styleVal.replace(reg, function (m, $1, $2) {
      const unit = $2 || 'px'
      return parseScale($1, unit)
    })
    return val
  }

  // otherwise
  return styleVal
}

export function autoPrefix (style: {}): {} {
  return addPrefix(style)
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
  const transformStr = elm.style.webkitTransform || elm.style.transform
  let styleObj = {}
  if (
    transformStr
    && !replace
    && transformStr.match(/(?: *(?:translate|rotate|scale)[^(]*\([^(]+\))+/i)) {
    styleObj = transformStr.trim().split(' ').reduce(function (pre, str) {
      ['translate', 'scale', 'rotate'].forEach(function (name) {
        if (new RegExp(name, 'i').test(str)) {
          pre[name] = str
        }
      })
      return pre
    }, {})
  }
  for (const key in style) {
    const val = style[key]
    if (val) {
      styleObj[key] = val
    }
  }
  const resStr = Object.keys(style).reduce(function (pre, key) {
    return pre + style[key] + ' '
  }, '')
  elm.style.webkitTransform = resStr
  elm.style.transform = resStr
}
