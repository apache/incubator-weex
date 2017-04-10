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
import { getThrottleLazyload, watchAppear } from '../utils'
import { tagBeforeCreate, tagMounted, tagBeforeUpdate, tagUpdated, tagBegin, tagEnd } from '../utils/perf'

const supportedEvents = [
  'click', 'longpress', 'appear', 'disappear'
  // 'touchstart', 'touchmove', 'touchend'
]

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

let warned = false
const notePage = 'https://github.com/MrRaindrop/weex/issues/14'
function warnProcessStyle () {
  if (!warned) {
    warned = true
    console.warn(`[vue-render] warn: should add loader config using $processStyle to enable`
      + ` inline styles's auto-prefixing. see ${notePage}. If already did it, please ignore this.`)
  }
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
    if (!weex._root) {
      weex._root = this.$root.$el
      weex._root.classList.add('weex-root')
    }
    if (!warned && !window._style_processing_added) {
      warnProcessStyle()
    }
    watchAppear(this)
    if (process.env.NODE_ENV === 'development') {
      tagMounted()
    }
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
  },

  methods: {
    _getTopContext () {
      let ctx = this
      let vnode = ctx.$options._parentVnode
      while (vnode) {
        ctx = vnode.context
        vnode = ctx.$options._parentVnode
      }
      return ctx
    },

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

    _createEventMap (extras = []) {
      const eventMap = {}
      supportedEvents.concat(extras).forEach(name => {
        eventMap[name] = event => this.$emit(name, event)
      })
      return eventMap
    },

    _fireLazyload (el) {
      if (process.env.NODE_ENV === 'development') {
        tagBegin('base._fireLazyload')
      }
      getThrottleLazyload(16)()
      if (process.env.NODE_ENV === 'development') {
        tagEnd('base._fireLazyload')
      }
    },

    _trimTextNodeChildren (children) {
      if (Array.isArray(children)) {
        return children.filter(vnode => !!vnode.tag)
      }
      return children
    }
  }
}
