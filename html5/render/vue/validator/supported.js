const supportedProps = {
}

const supportedStyles = {
  '@box-model': [
    'width', 'height',
    'padding-top', 'padding-bottom', 'padding-left', 'padding-right',
    'margin-top', 'margin-bottom', 'margin-left', 'margin-right'
  ],
  text: [
    '@box-model', 'position', 'background-color', 'color', 'opacity'
  ]
}

/**
 * Flatten a multiple dimension array.
 */
export function flatten (array) {
  return array.reduce((dist, item) => {
    return dist.concat(Array.isArray(item) ? flatten(item) : item)
  }, [])
}

/**
 * Check if the value is in the list.
 * @param {String} type
 * @param {String} value
 * @param {Object} dict
 */
export function checkSupported (type, value, dict = {}) {
  if (type && value && dict[type]) {
    return flatten(dict[type].map(k => dict[k] || k)).indexOf(value) !== -1
  }
  return true
}

/**
 * Check if the style is supported for the component.
 * @param {String} type
 * @param {String} style
 */
export function isSupportedStyle (type, style) {
  return checkSupported(type, style, supportedStyles)
}

/**
 * Check if the property is supported for the component.
 * @param {String} type
 * @param {String} property
 */
export function isSupportedProp (type, prop) {
  return checkSupported(type, prop, supportedProps)
}
