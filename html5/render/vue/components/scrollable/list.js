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
import { scrollable, list as listMixin } from './mixins'

function getList (weex) {
  const {
    extractComponentStyle,
    createEventMap
  } = weex

  return {
    name: 'weex-list',
    mixins: [scrollable, listMixin],
    computed: {
      wrapperClass () {
        const classArray = ['weex-list', 'weex-list-wrapper', 'weex-ct']
        this._refresh && classArray.push('with-refresh')
        this._loading && classArray.push('with-loading')
        return classArray.join(' ')
      }
    },

    methods: {
      createChildren (h) {
        const slots = this.$slots.default || []
        this._cells = slots.filter(vnode => {
          if (!vnode.tag || !vnode.componentOptions) return false
          return true
        })
        return [
          h('article', {
            ref: 'inner',
            staticClass: 'weex-list-inner weex-ct'
          }, this._cells)
        ]
      }
    },

    render (createElement) {
      this.weexType = 'list'

      this.$nextTick(() => {
        this.updateLayout()
      })

      return createElement('main', {
        ref: 'wrapper',
        attrs: { 'weex-type': 'list' },
        staticClass: this.wrapperClass,
        on: createEventMap(this, {
          scroll: this.handleListScroll,
          touchstart: this.handleTouchStart,
          touchmove: this.handleTouchMove,
          touchend: this.handleTouchEnd
        }),
        staticStyle: extractComponentStyle(this)
      }, this.createChildren(createElement))
    }
  }
}

export default {
  init (weex) {
    weex.registerComponent('list', getList(weex))
  }
}
