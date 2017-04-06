// @flow

import { isElementVisible } from './component'
import { createEvent, dispatchEvent } from './event'
import { throttle } from './func'
import { tagImg } from './perf'

const SCREEN_REC_LIMIT = 3  // just record the first 3 times for screen-render finishing.
let doRecord = true

function preLoadImg (src: string,
    loadCallback: ?(Event) => void,
    errorCallback: ?(Event) => void): void {
  const img = new Image()
  img.onload = loadCallback ? loadCallback.bind(img) : null
  img.onerror = errorCallback ? errorCallback.bind(img) : null
  img.src = src
}

export function applySrc (item: HTMLElement, src: ?string, placeholderSrc: ?string): void {
  if (!src) { return }
  function finallCb () {
    item.removeAttribute('img-src')
    if (doRecord) {
      if (window._weex_perf.renderTime.length < SCREEN_REC_LIMIT) {
        tagImg() // tag lastest img onload time.
      }
      else {
        doRecord = false
      }
    }
  }
  preLoadImg(src, function (evt) {
    item.style.backgroundImage = `url(${src || ''})`
    const { width: naturalWidth, height: naturalHeight } = this
    dispatchEvent(item, createEvent(item, 'load', { naturalWidth, naturalHeight }))
    finallCb()
  }, function (evt) {
    dispatchEvent(item, createEvent(item, 'error'))
    if (placeholderSrc) {
      preLoadImg(placeholderSrc, function () {
        item.style.backgroundImage = `url(${placeholderSrc || ''})`
      })
    }
    finallCb()
  })
}

export function fireLazyload (el: Array<HTMLElement> | HTMLElement | null, ignoreVisibility: ?boolean): void {
  if (Array.isArray(el)) {
    return el.forEach(ct => fireLazyload(ct))
  }
  el = el || document.body
  if (!el) { return }
  const imgs: NodeList = (el || document.body).querySelectorAll('[img-src]')
  for (let i: number = 0; i < imgs.length; i++) {
    const img = imgs[i]
    if (typeof ignoreVisibility === 'boolean' && ignoreVisibility) {
      applySrc(img, img.getAttribute('img-src'), img.getAttribute('img-placeholder'))
    }
    else if (isElementVisible(img, el)) {
      applySrc(img, img.getAttribute('img-src'), img.getAttribute('img-placeholder'))
    }
    // In somecases there are images out of the screen in x-axis. There
    // should not be a break point in these cases.
    // else {
    //   // alreay out of view, no need to compare any more.
    //   break
    // }
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
export function getThrottleLazyload (wait: number = 16, el: HTMLElement | null = document.body) {
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
