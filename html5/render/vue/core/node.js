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

/**
 * get listeners from on config and v-on binding.
 * v-on binding has a priority over on config.
 * @param {vnode} vnode
 * @param {String} evt: event name.
 */
function getListeners (vnode, evt) {
  const handlers = []
  while (vnode) {
    if (vnode.data && vnode.data.on) {
      const handler = vnode.data.on[evt]
      handler && handlers.push(handler)
    }
    if (vnode.componentOptions && vnode.componentOptions.listeners) {
      const handler = vnode.componentOptions.listeners[evt]
      handler && handlers.push(handler)
    }
    vnode = vnode.parent
  }
  return handlers
}

const supportedEvents = [
  'click', 'longpress', 'appear', 'disappear',
  // 'touchstart', 'touchmove', 'touchend',
  'panstart', 'panmove', 'panend', 'swipe', 'longpress'
]
/**
 * emit native events to enable v-on.
 * @param {VComponent} context: which one to emit a event on.
 * @param {array} extras: extra events.
 */
export function createEventMap (context, extras = []) {
  const eventMap = {}
  supportedEvents.concat(extras).forEach(name => {
    eventMap[name] = function (e) {
      // no original bubbling.
      e.stopPropagation()
      // but should trigger the closest parent which has bound the
      // event handler.
      let vm = context
      while (vm) {
        const ons = getListeners(vm.$vnode, name)
        const len = ons.length
        let idx = 0
        while (idx < len) {
          let on = ons[idx]
          if (on && on.fn) {
            on = on.fn
          }
          on && on.call(vm, e)
          idx++
        }

        // once a parent node (or self node) has triggered the handler,
        // then it stops bubble immediately.
        if (len > 0) {
          return
        }
        vm = vm.$parent
      }
    }
  })
  return eventMap
}
