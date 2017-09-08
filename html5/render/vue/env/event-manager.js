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
      const el = e.target
      let vm = el.__vue__
      let disposed = false
      let evtName = e.type
      /**
       * take full control of redirection of <a> element.
       */
      if (evtName === 'click') {
        // use '_triggered' to control bubbles event.
        e._triggered = { target: vm.$el }
        e.preventDefault()
        return
      }

      if (evtName === 'tap' && e._for !== 'weex') {
        return
      }
      else if (evtName === 'tap') {
        evtName = 'click'
      }

      while (vm) {
        const vnode = vm._vnode || vm.$vnode
        const elm = vm.$el
        const ons = getListeners(vnode, evtName)
        const len = ons && ons.length

        if (len > 0) {
          for (let i = 0; i < len; i++) {
            const handler = ons[i]
            const newEvt = evtName === 'click'
              ? createEvent(el, evtName)
              : e
            applyFns(handler.fns, newEvt)
          }
          e._triggered = { target: vm.$el }
          disposed = true
        }

        /**
         * if the handler is binding on a <a> element, should trigger
         * the handler first and then jump to href.
         * NOTE: if target==='_blank' then do no jumping and leave it
         * to users binding handlers for further disposing.
         */
        if (evtName === 'click' && isANode(elm)) {
          const href = elm.getAttribute('href')
          const voidHrefReg = /^\s*javascript\s*:\s*void\s*(?:\(\s*0\s*\)|0)\s*;?\s*$/
          const prevent = elm.getAttribute('prevent')
          disposed = true
          /**
           * Give the chance to the listeners binding on doc or doc.body for
           * handling the a-jump.
           * Should set a _should_intercept_a_jump function on window to test
           * whether we should intercept the a-jump.
           */
          if (window._should_intercept_a_jump && window._should_intercept_a_jump(elm)
            || href.match(voidHrefReg)
            || prevent === '' || prevent === 'true') {
            // do nothing. leave it to the intercept handler.
          }
          else if (href) {
            location.href = href
          }
          else if (process.env.NODE_ENV === 'development') {
            console.warn('[vue-render] If you want to use the A tag jump, set the href attribute')
          }
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
