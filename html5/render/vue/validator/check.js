const supportedProperties = {
  '@common': [
    'id', 'ref', 'style', 'class', 'append'
  ]
}

const supportedStyles = {
  '@box-model': [
    'width', 'height', 'position',
    'padding-top', 'padding-bottom', 'padding-left', 'padding-right',
    'margin-top', 'margin-bottom', 'margin-left', 'margin-right'
  ],
  '@border': [
    'border-style', 'border-width', 'border-color', 'border-radius',
    'border-top-style', 'border-right-style', 'border-bottom-style', 'border-left-style',
    'border-top-width', 'border-right-width', 'border-bottom-width', 'border-left-width',
    'border-top-color', 'border-right-color', 'border-bottom-color', 'border-left-color',
    'border-top-left-radius', 'border-top-right-radius', 'border-bottom-left-radius', 'border-bottom-right-radius'
  ],
  '@flexbox': [
    'display', 'flex', 'flex-direction', 'justify-content', 'align-items', 'flex-wrap'
  ],
  '@font': [
    'font-size', 'font-weight', 'font-style', 'font-family'
  ],
  '@colors': [
    'color', 'background-color', 'opacity'
  ],
  text: [
    '@box-model', '@border', '@flexbox', '@font', '@colors',
    'text-align', 'text-decoration', 'text-overflow'
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
  return checkSupported(type, prop, supportedProperties)
}
