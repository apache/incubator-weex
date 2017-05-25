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
import { throttle, extend } from './func'
import { createEvent } from './event'

export function getParentScroller (vnode) {
  if (!vnode) return null
  if (vnode.weexType === 'scroller' || vnode.weexType === 'list') {
    return vnode
  }
  return getParentScroller(vnode.$parent)
}

export function hasIntersection (rect, ctRect) {
  return (rect.left < ctRect.right && rect.right > ctRect.left)
    && (rect.top < ctRect.bottom && rect.bottom > ctRect.top)
}

/**
 * isElementVisible
 * @param  {HTMLElement}  el    a dom element.
 * @param  {HTMLElement}  container  optional, the container of this el.
 */
export function isElementVisible (el, container) {
  if (!el.getBoundingClientRect) { return false }
  const bodyRect = {
    top: 0,
    left: 0,
    bottom: window.innerHeight,
    right: window.innerWidth
  }
  const ctRect = (container === document.body)
    ? bodyRect : container
    ? container.getBoundingClientRect() : bodyRect
  return hasIntersection(
    el.getBoundingClientRect(),
    ctRect)
}

export function isComponentVisible (component) {
  if (component.$el) {
    const scroller = getParentScroller(component)
    if (scroller && scroller.$el) {
      return hasIntersection(
        component.$el.getBoundingClientRect(),
        scroller.$el.getBoundingClientRect()
      )
    }
    else {
      return isElementVisible(component.$el)
    }
  }
  return false
}

// to trigger the appear/disappear event.
function triggerEvent (elm, handlers, isShow, dir) {
  const evt = isShow ? 'appear' : 'disappear'
  let listener = handlers[evt]
  if (listener && listener.fn) {
    listener = listener.fn
  }
  if (listener) {
    listener(createEvent(elm, evt, {
      direction: dir
    }))
  }
}

export function watchAppear (context) {
  if (!context || !context.$el) return null
  const el = context.$el
  context.$nextTick(() => {
    if ((context.$options && context.$options._parentListeners)
      || (context.$vnode && context.$vnode.data && context.$vnode.data.on)) {
      const on = extend({}, context.$options._parentListeners, context.$vnode.data.on)
      if (on.appear || on.disappear) {
        const scroller = getParentScroller(context)
        let isWindow = false
        let container = window
        if (scroller && scroller.$el) {
          container = scroller.$el
        }
        else {
          isWindow = true
        }
        const visible = isElementVisible(el, isWindow ? document.body : container)
        if (context._visible !== visible) {
          context._visible = visible
          // if the component hasn't appeared for once yet, then it shouldn't trigger
          // a disappear event at all.
          if (context._appearedOnce) {
            triggerEvent(el, on, visible, null)
          }
          else if (visible) {
            context._appearedOnce = true
            triggerEvent(el, on, true, null)
          }
        }

        let lastScrollTop = container.scrollTop || window.pageYOffset
        // no need to watch the same vComponent again.
        if (!context._scrollWatched) {
          context._scrollWatched = true
          const handler = throttle(event => {
            const visible = isElementVisible(el, isWindow ? document.body : container)
            if (visible !== context._visible) {
              context._visible = visible
              const scrollTop = container.scrollTop || window.pageYOffset
              const dir = scrollTop < lastScrollTop
                ? 'down' : scrollTop > lastScrollTop
                ? 'up' : null
              triggerEvent(el, on, visible, dir)
              lastScrollTop = scrollTop
            }
          }, 25, true)

          container.addEventListener('scroll', handler, false)
        }
      }
    }
  })
}
