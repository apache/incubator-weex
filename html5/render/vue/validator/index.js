import * as styleValidator from './style'
import * as propValidator from './prop'
import { hyphenate, camelize } from '../utils'
import { isSupportedStyle, isSupportedProp } from './supported'

/**
 * Validate the styles of the component.
 * @param {String} type
 * @param {Object} styles
 */
export function validateStyles (type, styles = {}) {
  let isValidate = true
  for (const key in styles) {
    if (!isSupportedStyle(type, hyphenate(key))) {
      isValidate = false
      console.warn(`[Style Validator] <${type}> is not support to use the "${key}" style.`)
    }
    const validator = styleValidator[camelize(key)]
    if (validator && !validator(styles[key])) {
      isValidate = false
      console.warn(`[Style Validator] The style "${key}" is not support the "${styles[key]}" value.`)
    }
  }
  return isValidate
}

/**
 * Validate the properties of the component.
 * @param {String} type
 * @param {Object} props
 */
export function validateProps (type, props = {}) {
  let isValidate = true
  for (const key in props) {
    if (!isSupportedProp(type, hyphenate(key))) {
      isValidate = false
      console.warn(`[Property Validator] <${type}> is not support to use the "${key}" property.`)
    }
    const validator = propValidator[camelize(key)]
    if (validator && !validator(props[key])) {
      isValidate = false
      console.warn(`[Property Validator] The property "${key}" is not support the "${props[key]}" value.`)
    }
  }
  return isValidate
}
