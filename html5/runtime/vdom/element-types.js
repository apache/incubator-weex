import BasicElement from './element'
import { getListener } from './operation'

/**
 * A map which stores all type of elements.
 * @type {Object}
 */
export const elementTypes = {}

/**
 * Outer encapsulation for Element.
 * @param {string} type
 * @param {object} props
 */
export function Element (type, props) {
  const XElement = elementTypes[type]
  if (XElement) {
    return new XElement(props)
  }
  return new BasicElement(type, props)
}

/**
 * Register an extended element type with component methods.
 * @param  {string} type    component type
 * @param  {array}  methods a list of method names
 * @return {XElement}
 */
export function registerElement (type, methods) {
  // Skip when no special component methods.
  if (!methods || !methods.length) {
    elementTypes[type] = BasicElement
    return BasicElement
  }

  // Init constructor.
  const XElement = function (props) {
    BasicElement.call(this, type, props)
  }

  // Init prototype.
  XElement.prototype = Object.create(BasicElement.prototype)
  XElement.prototype.constructor = BasicElement

  // Add methods to prototype.
  for (const methodName in methods) {
    XElement.prototype[methodName] = function (...args) {
      const listener = getListener(this.docId)
      if (listener) {
        listener.callComponentMethod(this.ref, type, methodName, args)
      }
    }
  }

  // Add to element type map.
  elementTypes[type] = XElement

  return XElement
}

/**
 * Clear all element types. Only for testing.
 */
export function clearElementTypes () {
  for (const type in elementTypes) {
    delete elementTypes[type]
  }
}
