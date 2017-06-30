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
  getHeadStyleMap,
  getComponentStyle,
  extractComponentStyle
} from '../core'

import {
  normalizeStyle,
  camelizeKeys,
  extend
} from '../utils'

/**
 * get a beforeCreate hook, which has a mark to identify the hook function itself.
 */
function getIdentifiedBeforeCreate () {
  const disposed = {} // disposed components. Already scanned.
  function beforeCreate () {
    /**
     * get static class style map from document's styleSheets.
     * Weex.on will create a Vue instance. In this case we'll ignore it, since
     * it's not sure whether the scoped style has already attached to head or not.
     */
    const tagName = this.$options && this.$options._componentTag
    /**
     * For vue-loader ^11.3.x, there's no injectStyle function. The styleSheet
     * is already injected into the head. Just scan it.
     */
    if (this === this.$root && this.$options && !this._firstScanned) {
      this._firstScanned = true
      extend(weex._styleMap, getHeadStyleMap())
    }
    /**
     * For vue-loader ^12.0, the injectStyle function is hooked. We should scan
     * style map after the injectStyle hook called.
     */
    if (((this === this.$root && this.$options)
      || (tagName
      && !weex._components[tagName]
      && !disposed[tagName]))
      && !this._secondScanned) {
      disposed[tagName] = 1
      this._secondScanned = true
      const hooks = this.$options.beforeCreate
      const len = hooks.length
      let thisHookIdx = 0 // index of this hook in the hooks array.
      for (; thisHookIdx < len; thisHookIdx++) {
        if (hooks[thisHookIdx]._styleMixin) { break }
      }
      for (let i = thisHookIdx + 1; i < len; i++) {
        const func = hooks[i]
        if (func.name === 'injectStyle') {
          hooks[i] = function () {
            // call the original injectStyle hook.
            func.call(this)
            // scan the new appended styleSheet.
            extend(weex._styleMap, getHeadStyleMap())
            hooks[i] = func
          }
        }
      }
    }
  }
  beforeCreate._styleMixin = true
  return beforeCreate
}

export default {
  beforeCreate: getIdentifiedBeforeCreate(),

  methods: {
    $processStyle (style) {
      window._style_processing_added = true
      if (!style) {
        return
      }
      return normalizeStyle(camelizeKeys(style))
    },

    _extractComponentStyle () {
      return extractComponentStyle(this)
    },

    /**
     * get style from class, staticClass, style and staticStyle.
     * merge styles priority: high -> low
     *  1. data.style (bound style).
     *  2. data.staticStyle (inline styles).
     *  3. data.class style (bound class names).
     *  4. data.staticClass style (scoped styles or static classes).
     */
    _getComponentStyle (data) {
      return getComponentStyle(this)
    },

    _getParentRect () {
      const parentElm = this.$options._parentElm
      return parentElm && parentElm.getBoundingClientRect()
    }
  }
}
