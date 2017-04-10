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

/**
 * @fileOverview Impl of text component.
 *
 * Notes about the style 'height' and 'lines':
 * if the computed value of 'height' is bigger than 'lines', than the text will
 * be clipped according to the 'lines'. Otherwise, it'll be the 'height'.
 */

/**
 * Get text special styles (lines and text-overflow).
 */
function getTextSpecStyle (context = {}, ms = {}) {
  const lines = parseInt(ms.lines) || 0
  const overflow = ms['text-overflow'] || 'ellipsis'
  if (lines > 0) {
    return {
      overflow: 'hidden',
      'text-overflow': overflow,
      '-webkit-line-clamp': lines
    }
  }
}

export default {
  props: {
    lines: [Number, String],
    value: [String]
  },

  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('text', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    const ms = this._getComponentStyle(this.$vnode.data)
    return createElement('p', {
      attrs: { 'weex-type': 'text' },
      on: this._createEventMap(),
      staticClass: 'weex-text weex-el',
      staticStyle: getTextSpecStyle(this, ms)
    }, this.$slots.default || [this.value])
  }
}
