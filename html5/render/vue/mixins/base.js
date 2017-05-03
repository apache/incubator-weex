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
  getThrottleLazyload,
  watchAppear,
  extend
} from '../utils'

import {
  tagBeforeCreate,
  // tagMounted,
  tagRootMounted,
  tagFirstScreen,
  tagBeforeUpdate,
  tagUpdated,
  tagBegin,
  tagEnd
} from '../utils/perf'

// import { extractComponentStyle } from '../core'

const scrollableTypes = ['scroller', 'list']

let lazyloadWatched = false
function watchLazyload () {
  lazyloadWatched = true
  ; [
    'scroll'
    // 'transitionend',
    // 'webkitTransitionEnd',
    // 'animationend',
    // 'webkitAnimationEnd',
    // 'resize'
  ].forEach(evt => {
    window.addEventListener(evt, getThrottleLazyload(25, document.body))
  })
}

export default {
  beforeCreate () {
    if (!lazyloadWatched) {
      watchLazyload()
    }
    if (process.env.NODE_ENV === 'development') {
      tagBeforeCreate()
    }
  },

  mounted () {
    if (this.$options._componentTag === 'image') {
      global._has_image_in_first_screen = true
    }
    if (this === this.$root) {
      tagRootMounted()
      if (!global._has_image_in_first_screen) {
        tagFirstScreen()
      }
    }
    if (!weex._root) {
      weex._root = this.$root.$el
      weex._root.classList.add('weex-root')
    }

    // bind attrs to $el.
    let i, j
    if (this.$el && (i = j = this.$vnode) && (i = i.data) && (j = j.componentOptions)) {
      this.$el.attrs = extend({}, i.attrs, j.propsData)
    }

    watchAppear(this)
  },

  beforeUpdate () {
    if (process.env.NODE_ENV === 'development') {
      tagBeforeUpdate()
    }
  },

  updated () {
    if (process.env.NODE_ENV === 'development') {
      tagUpdated()
    }

    /**
     * During updating process, updateChildren will not process the render function of the
     * children's components. Therefore it is necessary to process children's static styles in
     * this hook func.
     */
    const children = this.$children
    if (children) {
      children.forEach((childVm) => {
        /**
         * Has to delete the previousVnode's staticStyle. Otherwise some style props may not
         * be attached on the dom, since the previous staticStyle val is equal with the new
         * updated style val.
         */
        let i
        if ((i = childVm._vnode) && (i = childVm._vnode.data)) {
          delete i.staticStyle
        }
        childVm._watcher.run()
      })
    }

    watchAppear(this)
  },

  methods: {
    _getScopeIds () {
      const arr = []
      let ctx = this
      let scopeId
      while (ctx) {
        scopeId = ctx.$options._scopeId
        scopeId && arr.push(scopeId)
        ctx = ctx.$options.parent
      }
      return arr
    },

    _getParentScroller () {
      let parent = this
      while (parent && scrollableTypes.indexOf(parent.$options._componentTag) <= -1) {
        parent = parent.$options.parent
      }
      return parent
    },

    _fireLazyload (el) {
      if (process.env.NODE_ENV === 'development') {
        tagBegin('base._fireLazyload')
      }
      getThrottleLazyload(16)()
      if (process.env.NODE_ENV === 'development') {
        tagEnd('base._fireLazyload')
      }
    }
  }
}
