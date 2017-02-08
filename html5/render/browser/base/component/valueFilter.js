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
        return val.replace(/\d*\.?\d+px/g, function (match) {
          return parseInt(parseFloat(match)) + 'px'
        })
      }
      if (key.match(/^border$/) && val.match(/^\d+(?:px)?\s+/)) {
        val = val.replace(/^(\d+(?:px)?)/, function ($0, $1) {
          const v = parseFloat($1)
          return v + 'px'
        })
      }
      return val
    }
  }
}
