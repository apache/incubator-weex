/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

import { extend, isArray } from '../utils'

/**
 * remove text nodes in the nodes array.
 * @param  {Array} nodes
 * @return {Array} nodes without text nodes.
 */
export function trimTextVNodes (vnodes) {
  if (isArray(vnodes)) {
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
  'longpress', 'appear', 'disappear',
  'touchstart', 'touchmove', 'touchend',
  'panstart', 'panmove', 'panend', 'swipe', 'longpress'
]

/**
 * is a element in a '<a>' tag?
 * @param {HTMLElement} el
 */
function isInANode (el) {
  let parent = el.parentNode
  while (parent && parent !== document.body) {
    if (parent.tagName.toLowerCase() === 'a') {
      return true
    }
    parent = parent.parentNode
  }
}

/**
 * emit native events to enable v-on.
 * @param {VComponent} context: which one to emit a event on.
 * @param {array | object} extras: extra events. You can pass in multiple arguments here.
 */
export function createEventMap (context, ...extras) {
  const eventMap = {}
  /**
   * Bind some original type event to your specified type event handler.
   * e.g. bind 'tap' event to 'click' event listener: bindFunc('tap')('click').
   * Or bind certian event with your specified handler: bindFunc('click', someFunction)
   */
  const bindFunc = (originalType) => {
    return listenTo => {
      let handler
      const evtName = originalType || listenTo
      if (typeof listenTo === 'function') {
        handler = listenTo
      }
      else if (typeof listenTo === 'string') {
        handler = function (e) {
          /**
           * allow original bubbling.
           * use '_triggered' to control actural bubbling.
           */
          if (e._triggered) {
            return
          }
          // but should trigger the closest parent which has bound the
          // event handler.
          let vm = context
          while (vm) {
            const ons = getListeners(vm.$vnode, listenTo)
            const len = ons.length
            if (len > 0) {
              let idx = 0
              while (idx < len) {
                let on = ons[idx]
                if (on && on.fn) {
                  on = on.fn
                }
                let evt = e
                if (originalType && evtName !== listenTo) {
                  evt = extend({}, { type: listenTo })
                  // weex didn't provide these two methods for event object.
                  delete event.preventDefault
                  delete event.stopPropagation
                }
                on && on.call(vm, evt)
                idx++
              }
              // once a parent node (or self node) has triggered the handler, then
              // it stops bubbling immediately, and a '_triggered' object is set.
              e._triggered = {
                el: vm.$el
              }
              return
            }
            vm = vm.$parent
          }
        }
      }
      if (!eventMap[evtName]) {
        eventMap[evtName] = []
      }
      eventMap[evtName].push(handler)
    }
  }

  /**
   * 1. Dispatch default supported events directly to user's event listeners. These
   * listeners will be triggered before extras event handlers.
   */
  supportedEvents.forEach(bindFunc())

  /**
   * 2. component's extra event bindings. This is mostly for the needs of component's
   * own special behaviours. These handlers will be processed after the user's
   * corresponding event handlers.
   */
  if (extras) {
    const len = extras.length
    for (let i = 0; i < len; i++) {
      const extra = extras[i]
      if (isArray(extra)) {
        extra.forEach(bindFunc())
      }
      else if (typeof extra === 'object') {
        for (const key in extra) {
          bindFunc(key)(extra[key])
        }
      }
    }
  }

  /**
   * 3. special binding for click event, which should be a fastclick event without
   * 300ms latency.
   */
  bindFunc('tap')('click')
  /**
   * Special treatment for click event:
   * we already use tap to trigger click event, so the click event should:
   * 1. trigger none of any vm's click listeners.
   * 2. prevent default behaviour for a `<a>` element.
   * This means the click event should always be swallowed in silence.
   */
  bindFunc('click')(function (e) {
    if (e._triggered) {
      return
    }
    let vm = context
    while (vm) {
      const ons = getListeners(vm.$vnode, 'click')
      const len = ons.length
      if (len > 0 && vm.$el && isInANode(vm.$el)) {
        e.preventDefault()
        e._triggered = { el: vm.$el }
        return
      }
      vm = vm.$parent
    }
  })

  return eventMap
}
