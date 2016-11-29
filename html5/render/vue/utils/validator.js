import { hyphenate } from './shared'

/**
 * Validate the CSS color value.
 */
export function isCSSColor (value) {
  return /^[a-z]+$/i.test(value) // match color name
    || /^#([a-f0-9]{3}){1,2}$/i.test(value) // match #ABCDEF
    || /^rgb\s*\((\s*[0-9.]+\s*,){2}\s*[0-9.]+\s*\)/i.test(value) // match rgb(0,0,0)
    || /^rgba\s*\((\s*[0-9.]+\s*,){3}\s*[0-9.]+\s*\)/i.test(value) // match rgba(0,0,0,0)
}

/**
 * CSS property validators
 */
const propValidators = {
  position (value) {
    return ['absolute', 'relative'].indexOf(value) !== -1
  },
  opacity (value) {
    const v = parseFloat(value)
    return v >= 0 && v <= 1
  },
  color: isCSSColor,
  'background-color': isCSSColor
}

const supportedStyles = {
  '@shared': [
    'width', 'height',
    'padding-top', 'padding-bottom', 'padding-left', 'padding-right',
    'margin-top', 'margin-bottom', 'margin-left', 'margin-right'
  ],
  text: [
    'position', 'background-color', 'color', 'opacity'
  ]
}

/**
 * Check if the style is valid for the components
 * @param {String} type
 * @param {Object} styles
 */
export function validateStyles (type, styles = {}) {
  if (supportedStyles[type]) {
    const props = supportedStyles[type].concat(supportedStyles['@shared'])
    for (const p in styles) {
      const key = hyphenate(p)
      if (props.indexOf(key) !== -1) {
        console.log(key, styles[p])
        if (propValidators[key] && !propValidators[key](styles[p])) {
          console.warn(
            `[Style Validator] ${key} is not support the value "${styles[p]}"`
          )
          return false
        }
      }
      else {
        console.warn(`[Style Validator] <${type}> is not support to use "${p}"`)
        return false
      }
    }
  }
  return true
}

export function validateText (context, options = {}) {
  // TODO: validate the props
  // TODO: validate the children
  // if (context) {
  //   return true
  // }
  return true
}
