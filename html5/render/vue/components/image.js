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

let extractComponentStyle, createEventMap, extend

const IMG_NAME_BITS = 15

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

function download (url, callback) {
  function success () {
    callback && callback({
      success: true
    })
  }
  function fail (err) {
    callback && callback({
      success: false,
      errorDesc: err + ''
    })
  }
  try {
    let isDataUrl = false
    let parts
    let name
    if (url.match(/data:image\/[^;]+;base64,/)) {
      isDataUrl = true
      parts = url.split(',')
    }
    if (!isDataUrl) {
      name = url
        .replace(/\?[^?]+/, '')
        .replace(/#[^#]+/, '')
        .match(/([^/]+)$/)
    }
    else {
      name = parts[1].substr(0, IMG_NAME_BITS)
    }
    const aEl = document.createElement('a')
    aEl.href = url
    /**
     * Not all browser support this 'download' attribute. In these browsers it'll jump
     * to the photo url page and user have to longpress the photo to save it.
     */
    aEl.download = name
    const clickEvt = new Event('click', { bubbles: false })
    aEl.dispatchEvent(clickEvt)
    success()
  }
  catch (err) {
    fail(err)
  }
}

const image = {
  name: 'weex-image',
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

  methods: {
    save (callback) {
      download(this.src, callback)
    }
  },

  render (createElement) {
    const resizeStyle = getResizeStyle(this)
    const style = extractComponentStyle(this)
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

export default {
  init (weex) {
    extractComponentStyle = weex.extractComponentStyle
    createEventMap = weex.createEventMap
    extend = weex.utils.extend

    weex.registerComponent('image', image)
    weex.registerComponent('img', image)
  }
}
