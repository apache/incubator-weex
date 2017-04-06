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

const vendorsReg = /webkit-|moz-|o-|ms-/
export function hyphenateStyleKeys (obj) {
  const res = {}
  for (const key in obj) {
    const hk = hyphenate(key).replace(vendorsReg, function ($0) {
      return '-' + $0
    })
    res[hk] = obj[key]
  }
  return res
}

export function camelToKebab (name) {
  if (!name) { return '' }
  return name.replace(/([A-Z])/g, function (g, g1) {
    return `-${g1.toLowerCase()}`
  })
}

export function appendCss (css, cssId, replace) {
  let style = document.getElementById(cssId)
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

export function nextFrame (callback) {
  const runner = window.requestAnimationFrame
    || window.webkitRequestAnimationFrame
    || (cb => setTimeout(cb, 16))
  runner(callback)
}

export function toCSSText (object) {
  if (!object) { return }
  const obj = hyphenateStyleKeys(object)
  let cssText = ''
  for (const key in obj) {
    cssText += `${key}:${obj[key]};`
  }
  return cssText
}
