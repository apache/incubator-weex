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

import { extend } from '../utils'

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
  'longpress', 'appear', 'disappear',
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
  /**
   * bind name with evt event. e.g. bind 'click' with 'tap' event.
   */
  const bindFunc = (evt) => {
    return name => {
      const evtType = evt || name
      eventMap[evtType] = function (e) {
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
            on && on.call(vm,
              evtType === name ? e : extend({}, e, { type: name })
            )
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
    }
  }
  supportedEvents.concat(extras).forEach(bindFunc())
  bindFunc('tap')('click')
  return eventMap
}
