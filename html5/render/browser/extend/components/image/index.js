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
/* global lib */

'use strict'

import './image.css'

const DEFAULT_SIZE = 200
const RESIZE_MODES = ['stretch', 'cover', 'contain']
const DEFAULT_RESIZE_MODE = 'stretch'

let Atomic

/**
 * resize: 'cover' | 'contain' | 'stretch', default is 'stretch'
 * src: url
 * placeholder / place-holder: url
 */
const proto = {
  create () {
    const node = document.createElement('div')
    node.classList.add('weex-img')
    node.classList.add('weex-element')
    return node
  },

  clearAttr () {
    this.src = ''
    this.node.style.backgroundImage = ''
  }
}

const attr = {
  src: function (val) {
    if (!this.src) {
      this.src = lib.img.defaultSrc
      this.node.style.backgroundImage = 'url(' + this.src + ')'
    }
    this.enableLazyload(val)
  },

  placeholder: function (val) {
    this.node.dataset.placeholder = val
  },

  // alias for placeholder (place-holder)
  placeHolder: function (val) {
    return this.attr.placeholder.call(this, val)
  },

  resize: function (val) {
    if (RESIZE_MODES.indexOf(val) === -1) {
      val = 'stretch'
    }
    this.node.style.backgroundSize = val === 'stretch'
                                    ? '100% 100%'
                                    : val
  }
}

const style = {
  width: function (val) {
    val = parseFloat(val)
    if (val < 0 || isNaN(val)) {
      val = DEFAULT_SIZE
    }
    this.node.style.width = val + 'px'
  },

  height: function (val) {
    val = parseFloat(val)
    if (val < 0 || isNaN(val)) {
      val = DEFAULT_SIZE
    }
    this.node.style.height = val + 'px'
  }
}

const event = {
  load: {
    extra: function () {
      const { naturalWidth, naturalHeight } = this.node
      return {
        naturalWidth, naturalHeight
      }
    }
  }
}

function init (Weex) {
  Atomic = Weex.Atomic
  const extend = Weex.utils.extend

  function Image (data) {
    this.resize = DEFAULT_RESIZE_MODE
    Atomic.call(this, data)
  }
  Image.prototype = Object.create(Atomic.prototype)
  extend(Image.prototype, proto)
  extend(Image.prototype, { attr })
  extend(Image.prototype, {
    style: extend(Object.create(Atomic.prototype.style), style)
  })
  extend(Image.prototype, { event })

  Weex.registerComponent('image', Image)
}

export default { init }
