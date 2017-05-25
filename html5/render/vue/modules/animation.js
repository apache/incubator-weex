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
import {
  nextFrame,
  toCSSText,
  autoPrefix,
  camelizeKeys,
  normalizeStyle,
  isArray
} from '../utils'

function transitionOnce (vnode, config, callback) {
  if (isArray(vnode)) {
    if (process.env.NODE_ENV === 'development') {
      console.warn('[vue-render] the ref passed to animation.transitionOnce is a array.')
    }
    vnode = vnode[0]
  }

  const duration = config.duration || 0 // ms
  const timing = config.timingFunction || 'linear'
  const delay = config.delay || 0  // ms

  // TODO: parse transition properties
  const transitionValue = `all ${duration}ms ${timing} ${delay}ms`

  const dom = vnode.$el
  // trigger image lazyloading by force.
  dom && weex.utils.fireLazyload(dom, true)

  const transitionEndHandler = function (event) {
    event.stopPropagation()
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

  nextFrame(() => {
    dom.style.cssText
      += toCSSText(autoPrefix(normalizeStyle(camelizeKeys(config.styles))) || {})
  })
}

export default {
  /**
   * transition
   * @param  {String} vnode
   * @param  {Object} config
   * @param  {String} callback
   */
  transition (vnode, config, callback) {
    if (!config.styles) { return }
    return transitionOnce(vnode, config, () => {
      callback && callback()
    })
  }
}
