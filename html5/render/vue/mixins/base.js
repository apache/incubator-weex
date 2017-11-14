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
  triggerDisappear,
  extend
} from '../utils'

let lazyloadWatched = false
function watchLazyload () {
  lazyloadWatched = true
  ; [
    'scroll',
    // 'transitionend',
    // 'webkitTransitionEnd',
    // 'animationend',
    // 'webkitAnimationEnd',
    'resize'
  ].forEach(evt => {
    window.addEventListener(evt, getThrottleLazyload(25, document.body))
  })
  /**
   * In case the users use the body's overflow to scroll. Then the scroll
   * event would not be triggered on the window object but on the body.
   */
  document.body.addEventListener('scroll', getThrottleLazyload(25, document.body))
}

let warned = false
const notePage = 'https://gist.github.com/MrRaindrop/5a805a067146609e5cfd4d64d775d693#file-weex-vue-render-config-for-vue-loader-js'
function warnProcessStyle () {
  if (!warned) {
    warned = true
    const page = window._process_style_note_page || notePage
    console.warn(`[vue-render]: you should add vue-loader config with $processStyle to enable inline styles's `
      + `normalization. see ${page} If you already did this, please ignore this message.`)
  }
}

let idCnt = 0

export default {
  beforeCreate () {
    if (!lazyloadWatched) {
      watchLazyload()
    }
  },

  updated () {
    if (this._rootId) {
      const el = this.$el
      if (el.nodeType === 1
        && el.className.indexOf('weex-root') <= -1) {
        el.classList.add('weex-root')
        el.setAttribute('data-wx-root-id', this._rootId)
      }
    }

    const tagName = this.$options && this.$options._componentTag
    const metaUp = weex._meta.updated
    if (!metaUp[tagName]) {
      metaUp[tagName] = 0
    }
    metaUp[tagName]++
    /**
     * since the updating of component may affect the layout, the lazyloading should
     * be fired.
     */
    this._fireLazyload()
  },

  mounted () {
    const tagName = this.$options && this.$options._componentTag
    if (typeof weex._components[tagName] !== 'undefined') {
      weex._components[tagName]++
    }
    const metaMt = weex._meta.mounted
    if (!metaMt[tagName]) {
      metaMt[tagName] = 0
    }
    metaMt[tagName]++
    if (this === this.$root) {
      const rootId = `wx-root-${idCnt++}`
      if (!weex._root) {
        weex._root = {}
      }
      weex._root[rootId] = this
      this._rootId = rootId
      const el = this.$el
      if (el.nodeType !== 1) {
        return
      }
      el.classList.add('weex-root')
      el.setAttribute('data-wx-root-id', rootId)
      this._fireLazyload(el)
    }

    // give warning for not using $processStyle in vue-loader config.
    if (!warned && !window._style_processing_added) {
      warnProcessStyle()
    }

    // bind attrs to $el.
    let i, j
    if (this.$el && (i = j = this.$vnode) && (i = i.data) && (j = j.componentOptions)) {
      this.$el.attrs = extend({}, i.attrs, j.propsData)
    }
    watchAppear(this, true)
  },

  destroyed () {
    /**
     * if the destroyed element is above another panel with images inside, and the images
     * moved into the viewport, then the lazyloading should be triggered.
     */
    if (this._rootId) {
      delete weex._root[this._rootId]
      delete this._rootId
    }
    const tagName = this.$options && this.$options._componentTag
    if (typeof weex._components[tagName] !== 'undefined') {
      weex._components[tagName]--
    }
    const metaDs = weex._meta.destroyed
    if (!metaDs[tagName]) {
      metaDs[tagName] = 0
    }
    metaDs[tagName]++

    this._fireLazyload()
    triggerDisappear(this)
  },

  methods: {
    _fireLazyload (el) {
      getThrottleLazyload(25, el || document.body)()
    }
  }
}
