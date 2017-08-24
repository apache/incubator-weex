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

import { isArray } from '../utils'

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

/**
 * emit native events to enable v-on.
 * @param {VComponent} context: which one to emit a event on.
 * @param {array | object} events: extra events. You can pass in multiple arguments here.
 */
export function createEventMap (context, ...events) {
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
           * use '_triggered' to control actural bubbling (allow original bubbling).
           */
          if (e._triggered) {
            return
          }
          /**
           * trigger the closest parent which has bound event handlers.
           */
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
                on && on.call(vm, e)
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
   * component's extra event bindings. This is mostly for the needs of component's
   * own special behaviours. These handlers will be processed after the user's
   * corresponding event handlers.
   */
  if (events) {
    const len = events.length
    for (let i = 0; i < len; i++) {
      const extra = events[i]
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

  return eventMap
}
