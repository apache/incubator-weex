/**
 * remove text nodes in the nodes array.
 * @param  {Array} nodes
 * @return {Array} nodes without text nodes.
 */
export function trimTextVNodes (vnodes) {
  if (Array.isArray(vnodes)) {
    return vnodes.filter(vnode => !!vnode.tag)
  }
  return vnodes
}

const supportedEvents = [
  'click', 'longpress', 'appear', 'disappear'
  // 'touchstart', 'touchmove', 'touchend'
]
/**
 * emit native events to enable v-on.
 * @param {VComponent} context: which one to emit a event on.
 * @param {array} extras: extra events.
 */
export function createEventMap (context, extras = []) {
  const eventMap = {}
  supportedEvents.concat(extras).forEach(name => {
    eventMap[name] = event => context.$emit(name, event)
  })
  return eventMap
}
