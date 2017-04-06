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
  return cssText.replace(/(?:\/\*)[^*]*\*\//g, '')
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
