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
import { supportSticky } from '../../utils/style'
import { extractComponentStyle, createEventMap } from '../../core'

export default {
  data () {
    return {
      sticky: false,
      initTop: 0,
      placeholder: null,
      supportSticky: supportSticky()
    }
  },

  mounted () {
    this.initTop = this.$el.offsetTop
    this.placeholder = window.document.createElement('div')
  },

  updated () {
    if (!this.sticky) {
      this.initTop = this.$el.offsetTop
    }
  },

  methods: {
    addSticky () {
      this.sticky = true
      this.placeholder.style.display = 'block'
      this.placeholder.style.width = this.$el.offsetWidth + 'px'
      this.placeholder.style.height = this.$el.offsetHeight + 'px'
      this.$el.parentNode.insertBefore(this.placeholder, this.$el)
    },

    removeSticky () {
      this.sticky = false
      try {
        this.$el.parentNode.removeChild(this.placeholder)
      }
      catch (e) {
      }
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('header', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    this._renderHook()
    return createElement('html:header', {
      attrs: { 'weex-type': 'header' },
      on: createEventMap(this),
      ref: 'header',
      staticClass: 'weex-header weex-ct',
      class: { 'weex-sticky': this.sticky, 'weex-ios-sticky': this.supportSticky },
      staticStyle: extractComponentStyle(this)
    }, this.$slots.default)
  }
}
