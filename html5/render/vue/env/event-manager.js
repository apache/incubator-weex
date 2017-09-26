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

import config from '../config'
import { createEvent, supportsPassive } from '../utils'
import { applyFns } from '../core'

const gestureEvents = config.gestureEvents
const nativeEvents = ['click', 'touchstart', 'touchmove', 'touchend']
const needPassive = ['touchmove']

const events = gestureEvents.concat(nativeEvents)

/**
 * if el is a `<a>` element.
 * @param {HTMLElement} el
 */
function isANode (el) {
  return el.tagName.toLowerCase() === 'a'
}

function isInANode (el) {
  let parent = el.parentElement
  while (parent && parent !== document.body) {
    if (parent.tagName === 'A') { return true }
    parent = parent.parentElement
  }
  return false
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

let _inited = false
function _init (doc) {
  if (_inited) {
    return
  }
  if (!doc) {
    return
  }
  _inited = true
  const _sp = supportsPassive()
  events.forEach(function (evt) {
    /**
     * use capture for click handling, therefore there's a chance to handle
     * it before any other listeners binding on document or document.body.
     */
    const option =
      evt === 'click'
      ? true : needPassive.indexOf(evt) > -1 && _sp
      ? { passive: true } : false
    doc.addEventListener(evt, function (e) {
      let el = e.target
      let vm = el.__vue__
      while (!vm && el && el !== document.body) {
        el = el.parentElement
        vm = el && el.__vue__
      }
      if (!vm) {  // not a vue component.
        return
      }
      let disposed = false
      const evtName = e.type

      if (evtName === 'tap' && e._for !== 'weex') {
        return
      }

      while (vm) {
        const vnode = vm._vnode || vm.$vnode
        const elm = vm.$el
        const ons = getListeners(vnode, evtName === 'tap' ? 'click' : evtName)
        const len = ons && ons.length

        if (len > 0) {
          if (evtName !== 'click') {
            for (let i = 0; i < len; i++) {
              const handler = ons[i]
              const newEvt = evtName === 'tap'
                ? createEvent(el, 'click')
                : e
              newEvt._triggered = { target: elm }
              applyFns(handler.fns, newEvt)
            }
          }
          e._triggered = { target: elm }
          disposed = true
        }

        if (isANode(elm)
          && (evtName === 'click' || evtName === 'tap')) {
          const href = elm.getAttribute('href')
          const voidHrefReg = /^\s*javascript\s*:\s*void\s*(?:\(\s*0\s*\)|0)\s*;?\s*$/
          const prevent = elm.getAttribute('prevent')
          if (window._should_intercept_a_jump && window._should_intercept_a_jump(elm)) {
            // e._triggered should not be true since we left the intercepter to handle the event.
            e._triggered = false
            disposed = true
          }
          else if (href.match(voidHrefReg)
            || prevent === '' || prevent === 'true') {
            e._triggered = false
            e.preventDefault()
          }
          else {
            e._triggered = { target: elm }
            disposed = true // handled by default behavior for clicking on a element.
          }
        }

        /**
         * If the click handler is binding on a element inside a <a> element,
         * then should prevent default.
         */
        if (disposed && evtName === 'click' && isInANode(elm)) {
          e._triggered = { target: elm }
          e.preventDefault()
          return
        }

        if (disposed) {
          return
        }
        vm = vm.$parent
      }
    }, option)
  })
}

export default function init () {
  _init(document)
}
