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

// @flow

import { isElementVisible } from './component'
import { createEvent, dispatchEvent } from './event'
import { throttle } from './func'

function preLoadImg (src: string,
    loadCallback: ?(Event) => void,
    errorCallback: ?(Event) => void): void {
  const img = new Image()
  img.onload = loadCallback ? loadCallback.bind(img) : null
  img.onerror = errorCallback ? errorCallback.bind(img) : null
  img.src = src
}

export function applySrc (item: any, src: ?string, placeholderSrc: ?string): void {
  if (!src) { return }
  function finallCb () {
    delete item._src_loading
  }
  if (item._src_loading === src) {
    return
  }
  /**
   * 1. apply src immediately in case javscript blocks the image loading
   *  before next tick.
   */
  item.style.backgroundImage = `url(${src || ''})`
  item.removeAttribute('img-src')
  /**
   * 2. then load the img src with Image constructor (but would not post
   *  a request again), just to trigger the load event.
   */
  item._src_loading = src
  preLoadImg(src, function (evt) {
    item.style.backgroundImage = `url(${src || ''})`
    const { width: naturalWidth, height: naturalHeight } = this
    const params = {
      success: true,
      size: { naturalWidth, naturalHeight }
    }
    dispatchEvent(item, createEvent(item, 'load', params))
    finallCb()
  }, function (evt) {
    const params = {
      success: false,
      size: { naturalWidth: 0, naturalHeight: 0 }
    }
    dispatchEvent(item, createEvent(item, 'load', params))
    if (placeholderSrc) {
      preLoadImg(placeholderSrc, function () {
        item.style.backgroundImage = `url(${placeholderSrc || ''})`
      })
    }
    finallCb()
  })
}

function getCtScroller (el: any) {
  if (!el) { return }
  let scroller = el._ptScroller
  if (!scroller) {
    let pt = el.parentElement
    while (pt && pt !== document.body) {
      if ((pt.className + '' || '').match(/weex-list|weex-scroller|weex-waterfall/)) {
        scroller = pt
        break
      }
      pt = pt.parentElement
    }
    scroller = pt
    el._ptScroller = pt
  }
  return scroller
}

export function fireLazyload (el: Array<any> | any | null, ignoreVisibility: ?boolean): void {
  if (Array.isArray(el)) {
    return el.forEach(ct => fireLazyload(ct))
  }
  el = el || document.body
  if (!el) { return }
  let imgs: NodeList | Array<any> = (el || document.body).querySelectorAll('[img-src]')
  if (el.getAttribute('img-src')) { imgs = [el] }
  for (let i: number = 0; i < imgs.length; i++) {
    const img = imgs[i]
    if (typeof ignoreVisibility === 'boolean' && ignoreVisibility) {
      applySrc(img, img.getAttribute('img-src'), img.getAttribute('img-placeholder'))
    }
    else if (isElementVisible(img, getCtScroller(el))[0]) {
      applySrc(img, img.getAttribute('img-src'), img.getAttribute('img-placeholder'))
    }
  }
}

/**
 * cache a throttle lazyload function for every container element
 * once for different wait times separate.
 *   the architecture of this cache:
 *      cache: {
 *        el.id: {
 *          wait: throttledFunction () { ... }
 *        }
 *      }
 */
const cache = {}
let _uid: number = 1
export function getThrottleLazyload (wait: number = 16, el: any | null = document.body) {
  let id: number = +(el && el.dataset.throttleId)
  if (isNaN(id) || id <= 0) {
    id = _uid++
    el && el.setAttribute('data-throttle-id', id + '')
  }

  !cache[id] && (cache[id] = {})
  const throttled = cache[id][wait] ||
    (cache[id][wait] = throttle(
      fireLazyload.bind(this, el),
      parseFloat(wait),
      // true for callLastTime.
      // to trigger once more time after the last throttled function called with a little more delay.
      true)
    )
  return throttled
}
