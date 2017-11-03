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
import config from '../config'

export function getParentScroller (vm) {
  if (!vm) return null
  if (vm._parentScroller) {
    return vm._parentScroller
  }
  function _getParentScroller (parent) {
    if (!parent) { return }
    if (config.scrollableTypes.indexOf(parent.weexType) > -1) {
      vm._parentScroller = parent
      return parent
    }
    return _getParentScroller(parent.$parent)
  }
  return _getParentScroller(vm.$parent)
}

function horizontalBalance (rect, ctRect) {
  return rect.left < ctRect.right && rect.right > ctRect.left
}

function verticalBalance (rect, ctRect) {
  return rect.top < ctRect.bottom && rect.bottom > ctRect.top
}

/**
 * return a data array with two boolean value, which are:
 * 1. visible in current ct's viewport.
 * 2. visible with offset in current ct's viewport.
 */
export function hasIntersection (rect, ctRect, dir, offset) {
  dir = dir || 'up'
  const isHorizontal = dir === 'left' || dir === 'right'
  const isVertical = dir === 'up' || dir === 'down'
  if (isHorizontal && !verticalBalance(rect, ctRect)) {
    return [false, false]
  }
  if (isVertical && !horizontalBalance(rect, ctRect)) {
    return [false, false]
  }
  offset = parseInt(offset || 0) * weex.config.env.scale
  switch (dir) {
    case 'up':
      return [
        rect.top < ctRect.bottom && rect.bottom > ctRect.top,
        rect.top < ctRect.bottom + offset && rect.bottom > ctRect.top - offset
      ]
    case 'down':
      return [
        rect.bottom > ctRect.top && rect.top < ctRect.bottom,
        rect.bottom > ctRect.top - offset && rect.top < ctRect.bottom + offset
      ]
    case 'left':
      return [
        rect.left < ctRect.right && rect.right > ctRect.left,
        rect.left < ctRect.right + offset && rect.right > ctRect.left - offset
      ]
    case 'right':
      return [
        rect.right > ctRect.left && rect.left < ctRect.right,
        rect.right > ctRect.left - offset && rect.left < ctRect.right + offset
      ]
  }
}

/**
 * isElementVisible
 * @param  {HTMLElement}  el    a dom element.
 * @param  {HTMLElement}  container  optional, the container of this el.
 */
export function isElementVisible (el, container, dir, offset) {
  if (!el.getBoundingClientRect) { return false }
  const bodyRect = {
    top: 0,
    left: 0,
    bottom: window.innerHeight,
    right: window.innerWidth
  }
  const ctRect = (container === window || container === document.body)
    ? bodyRect : container
    ? container.getBoundingClientRect() : bodyRect
  return hasIntersection(el.getBoundingClientRect(), ctRect, dir, offset)
}

// to trigger the appear/disappear event.
function triggerEvent (elm, handlers, evt, dir) {
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

/**
 * get all event listeners. including bound handlers in all parent vnodes.
 */
export function getEventHandlers (context) {
  let vnode = context.$vnode
  const handlers = {}
  const attachedVnodes = []
  while (vnode) {
    attachedVnodes.push(vnode)
    vnode = vnode.parent
  }
  attachedVnodes.forEach(function (vnode) {
    const parentListeners = vnode.componentOptions && vnode.componentOptions.listeners
    const dataOn = vnode.data && vnode.data.on
    extend(handlers, parentListeners, dataOn)
  })
  return handlers
}

function getAppearOffset (el) {
  return el && el.getAttribute('appear-offset')
}

function checkHandlers (handlers) {
  return [
    !!(handlers.appear || handlers.disappear),
    !!(handlers.offsetAppear || handlers.offsetDisappear)
  ]
}

/**
 * Watch element's visibility to tell whether should trigger a appear/disappear
 * event in scroll handler.
 */
export function watchAppear (context, fireNow) {
  const el = context && context.$el
  if (!el || el.nodeType !== 1) { return }
  const appearOffset = getAppearOffset(el)

  const handlers = getEventHandlers(context)
  const checkResults = checkHandlers(handlers)
  // no appear or offsetAppear handler was bound.
  if (!checkResults[0] && !checkResults[1]) {
    return
  }

  let isWindow = false
  let container = window
  const scroller = getParentScroller(context)
  if (scroller && scroller.$el) {
    container = scroller.$el
  }
  else {
    isWindow = true
  }

  if (fireNow) {
    const visibleData = isElementVisible(el, container, null, appearOffset)
    detectAppear(context, visibleData, null)
  }

  // add current vm to the container's appear watch list.
  if (!container._watchAppearList) {
    container._watchAppearList = []
  }
  container._watchAppearList.push(context)

  /**
   * Code below will only exec once for binding scroll handler for parent container.
   */
  if (container._scrollWatched) {
    return
  }
  container._scrollWatched = true
  const scrollHandler = throttle(event => {
    /**
     * detect scrolling direction.
     * direction only support up & down yet.
     * TODO: direction support left & right.
     */
    const scrollTop = isWindow ? window.pageYOffset : container.scrollTop
    const preTop = container._lastScrollTop
    container._lastScrollTop = scrollTop
    const dir = (scrollTop < preTop
      ? 'down' : scrollTop > preTop
      ? 'up' : container._prevDirection) || null
    container._prevDirection = dir
    const watchAppearList = container._watchAppearList || []
    const len = watchAppearList.length
    for (let i = 0; i < len; i++) {
      const vm = watchAppearList[i]
      const el = vm.$el
      const appearOffset = getAppearOffset(el)
      const visibleData = isElementVisible(el, container, dir, appearOffset)
      detectAppear(vm, visibleData, dir)
    }
  }, 25, true)
  container.addEventListener('scroll', scrollHandler, false)
  /**
   * In case the users use the body's overflow to scroll. Then the scroll
   * event would not be handled on the window object but on the body.
   */
  if (isWindow) {
    document.body.addEventListener('scroll', scrollHandler, false)
  }
}

/**
 * trigger a disappear event.
 */
export function triggerDisappear (context) {
  return detectAppear(context, [false, false])
}

/**
 * decide whether to trigger a appear/disappear event.
 * @param {VueComponent} context
 * @param {boolean} visible
 * @param {string} dir
 */
export function detectAppear (context, visibleData, dir = null, appearOffset) {
  const el = context && context.$el
  const [visible, offsetVisible] = visibleData
  if (!el) { return }
  const handlers = getEventHandlers(context)
  /**
   * No matter it's binding appear/disappear or both of them. Always
   * should test it's visibility and change the context/._visible.
   * If neithor of them was bound, then just ignore it.
   */
  /**
   * if the component hasn't appeared for once yet, then it shouldn't trigger
   * a disappear event at all.
   */
  if (context._appearedOnce || visible) {
    if (context._visible !== visible) {
      if (!context._appearedOnce) {
        context._appearedOnce = true
      }
      context._visible = visible
      triggerEvent(el, handlers, visible ? 'appear' : 'disappear', dir)
    }
  }
  if (context._offsetAppearedOnce || offsetVisible) {
    if (context._offsetVisible !== offsetVisible) {
      if (!context._offsetAppearedOnce) {
        context._offsetAppearedOnce = true
      }
      context._offsetVisible = offsetVisible
      triggerEvent(el, handlers, offsetVisible ? 'offsetAppear' : 'offsetDisappear', dir)
    }
  }
}
