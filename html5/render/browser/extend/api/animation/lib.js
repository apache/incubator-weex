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

/**
 * config:
 *   - styles
 *   - duration [Number] milliseconds(ms)
 *   - timingFunction [string]
 *   - dealy [Number] milliseconds(ms)
 */
export function transitionOnce (comp, config, callback) {
  const styles = config.styles || {}
  const duration = config.duration || 1000 // ms
  const timingFunction = config.timingFunction || 'ease'
  const delay = config.delay || 0  // ms
  const transitionValue = 'all ' + duration + 'ms '
      + timingFunction + ' ' + delay + 'ms'
  const dom = comp.node
  const transitionEndHandler = function (e) {
    e.stopPropagation()
    dom.removeEventListener('webkitTransitionEnd', transitionEndHandler)
    dom.removeEventListener('transitionend', transitionEndHandler)
    dom.style.transition = ''
    dom.style.webkitTransition = ''
    callback()
  }
  dom.style.transition = transitionValue
  dom.style.webkitTransition = transitionValue
  dom.addEventListener('webkitTransitionEnd', transitionEndHandler)
  dom.addEventListener('transitionend', transitionEndHandler)
  comp.updateStyle(styles)
}
