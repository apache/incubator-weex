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
  getParentScroller
} from '../utils'

export default {
  destroyed () {
    if (!this._stickyAdded) { return }
    const scroller = getParentScroller(this)
    if (!scroller) { return }
    delete scroller._stickyChildren[this._uid]
  },

  methods: {
    _addSticky () {
      const el = this.$el
      if (!el || el.nodeType === 1) {
        if (process.env.NODE_ENV === 'development') {
          console.error(`[vue-render] $el doesn't exist to add sticky.`)
        }
        return
      }
      el.classList.add('sticky')
      if (!this._placeholder) {
        this._placeholder = el.cloneNode(true)
      }
      this._placeholder.style.display = 'block'
      this._placeholder.style.width = this.$el.offsetWidth + 'px'
      this._placeholder.style.height = this.$el.offsetHeight + 'px'
      el.parentNode.insertBefore(this._placeholder, this.$el)
    },

    _removeSticky () {
      const el = this.$el
      if (!el || el.nodeType === 1) {
        if (process.env.NODE_ENV === 'development') {
          console.error(`[vue-render] $el doesn't exist to remove sticky.`)
        }
        return
      }
      el.classList.remove('sticky')
      if (this._placeholder) {
        this._placeholder.parentNode.removeChild(this._placeholder)
      }
      this._placeholder = null
    }
  }
}
