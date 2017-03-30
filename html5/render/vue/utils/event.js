// import { extend } from './func'

function extend (to, ...args) {
  if (!args || args.length <= 0) { return to }
  args.forEach(from => {
    if (typeof from !== 'object') { return }
    for (const key in from) {
      to[key] = from[key]
    }
  })
  return to
}

/**
 * Create Event.
 * @param {DOMString} type
 * @param {Object} props
 */
export function createEvent (context, type, props) {
  const event = new Event(type, { bubbles: false })
  // event.preventDefault()
  // event.stopPropagation()

  extend(event, props)

  Object.defineProperty(event, 'target', {
    enumerable: true,
    value: context || null
  })

  return event
}

/**
 * Create Custom Event.
 * @param {DOMString} type
 * @param {Object} props
 */
export function createCustomEvent (context, type, props) {
  // compatibility: http://caniuse.com/#search=customevent
  // const event = new CustomEvent(type)
  const event = document.createEvent('CustomEvent')
  event.initCustomEvent(type, false, true, {})
  // event.preventDefault()
  // event.stopPropagation()

  extend(event, props)

  // TODO: event.target is readonly
  Object.defineProperty(event, 'target', {
    enumerable: true,
    value: context || null
  })

  return event
}

/**
 * dispatch a event on a dom element.
 * @param  {HTMLElement} dom
 * @param  {Event} event
 */
export function dispatchEvent (dom, event) {
  dom.dispatchEvent(event)
}

export function mapFormEvents (context) {
  const eventMap = {}
  ;['input', 'change', 'focus', 'blur'].forEach(type => {
    eventMap[type] = event => {
      if (context.$el) {
        event.value = context.$el.value
      }
      context.$emit(type, event)
    }
  })
  return eventMap
}

export default {
  mapFormEvents
}
