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
'use strict'

import { throttle } from '../utils'

const watchedComponents = []
let listened = false
let scrollY = 0

function needWatch (component) {
  const events = component.data.event
  if (events
      && (events.indexOf('appear') !== -1
        || events.indexOf('disappear') !== -1)) {
    return true
  }
  return false
}

export function watchIfNeeded (component) {
  if (needWatch(component)) {
    watchedComponents.push(component)
    if (!listened) {
      listened = true
      const handler = throttle(onScroll, 100)
      window.addEventListener('scroll', handler, false)
    }
  }
}

export function isComponentInWindow (component) {
  const rect = component.node.getBoundingClientRect()
  return rect.right > 0 && rect.left < window.innerWidth &&
         rect.bottom > 0 && rect.top < window.innerHeight
}

export function hasIntersection (rect, ctRect) {
  return (rect.left < ctRect.right && rect.right > ctRect.left)
    && (rect.top < ctRect.bottom && rect.bottom > ctRect.top)
}

export function isComponentAppear (component) {
  // NOTE: no more support embeded scrollers.
  const parentScroller = component.getParentScroller()
  if (!parentScroller) {
    return isComponentInWindow(component)
  }
  return isComponentInWindow(component)
    && hasIntersection(
      component.node.getBoundingClientRect(),
      parentScroller.node.getBoundingClientRect())
}

function onScroll (e) {
  let direction
  // NOTE: this condition strongly relies on the scroller's implementation.
  if (e.originalType === 'scrolling') {
    direction = e.direction
  }
  else {
    // NOTE: only VERTICAL window scroll can be detected.
    const y = window.scrollY
    direction = y >= scrollY ? 'up' : 'down'
    scrollY = y
  }
  const len = watchedComponents.length
  for (let i = 0; i < len; i++) {
    const component = watchedComponents[i]
    const appear = isComponentAppear(component)
    if (appear) {
      component.dispatchEvent('appear', { direction })
    }
    else if (!appear) {
      component.dispatchEvent('disappear', { direction })
    }
  }
}
