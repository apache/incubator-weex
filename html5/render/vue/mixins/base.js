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
const notePage = 'https://gist.github.com/MrRaindrop/5a805a067146609e5cfd4d64d775d693#file-weex-vue-render-config-for-vue-loader-js'
function warnProcessStyle () {
  if (!warned) {
    warned = true
    const page = window._process_style_note_page || notePage
    console.warn(`[vue-render]: you should add vue-loader config with $processStyle to enable inline styles's `
      + `normalization. see ${page} If you already did this, please ignore this message.`)
  }
}

export default {
  beforeCreate () {
    if (!lazyloadWatched) {
      watchLazyload()
    }
  },

  mounted () {
    if (!weex._root) {
      weex._root = this.$root.$el
      weex._root.classList.add('weex-root')
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
    triggerDisappear(this)
  },

  methods: {
    _fireLazyload (el) {
      getThrottleLazyload(25)()
    }
  }
}
