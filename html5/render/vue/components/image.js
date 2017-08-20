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

import { extractComponentStyle, createEventMap } from '../core'
import { extend } from '../utils'

const _css = `
.weex-image, .weex-img {
  background-repeat: no-repeat;
  background-position: 50% 50%;
}
`
/**
 * get resize (stetch|cover|contain) related styles.
 */
function getResizeStyle (context) {
  const stretch = '100% 100%'
  const resize = context.resize || stretch
  const bgSize = ['cover', 'contain', stretch].indexOf(resize) > -1 ? resize : stretch
  // compatibility: http://caniuse.com/#search=background-size
  return { 'background-size': bgSize }
}

function preProcessSrc (context, url, mergedStyle) {
  // somehow the merged style in _prerender hook is gone.
  // just return the original src.
  if (!mergedStyle || !mergedStyle.width || !mergedStyle.height) {
    return url
  }
  const { width, height } = mergedStyle
  return context.processImgSrc && context.processImgSrc(url, {
    width: parseFloat(width),
    height: parseFloat(height),
    quality: context.quality,
    sharpen: context.sharpen,
    original: context.original
  }) || url
}

export default {
  props: {
    src: String,
    placeholder: String,
    resize: String,
    quality: String,
    sharpen: String,
    original: [String, Boolean]
  },

  updated () {
    this._fireLazyload()
  },

  mounted () {
    this._fireLazyload()
  },

  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('image', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    // const style = this._normalizeInlineStyles(this.$vnode.data)
    const resizeStyle = getResizeStyle(this)
    const style = extractComponentStyle(this)
    this._renderHook()
    return createElement('figure', {
      attrs: {
        'weex-type': 'image',
        'img-src': preProcessSrc(this, this.src, style),
        'img-placeholder': preProcessSrc(this, this.placeholder, style)
      },
      on: createEventMap(this, ['load', 'error']),
      staticClass: 'weex-image weex-el',
      staticStyle: extend(style, resizeStyle)
    })
  },
  _css
}
