// @flow

import { extend } from '../utils/func'

/**
 * viewport priority:
 *
 * 1. meta weex-viewport (developer custom)
 * 2. setViewport(config) := config.width (private code) @deprecated
 * 3. process.env.VIEWPORT_WIDTH (buid time)
 *
 */
let isInited = false
const DEFAULT_VIEWPORT_WIDTH = 750

/**
 * get viewport width from weex-viewport meta.
 */
const envViewportWidth = parseInt(process.env.VIEWPORT_WIDTH)
let width: number = !isNaN(envViewportWidth) && envViewportWidth > 0
  ? envViewportWidth
  : DEFAULT_VIEWPORT_WIDTH

let wxViewportMeta = document.querySelector('meta[name="weex-viewport"]')
const metaWidth = wxViewportMeta && parseInt(wxViewportMeta.getAttribute('content'))
if (metaWidth && !isNaN(metaWidth) && metaWidth > 0) {
  width = metaWidth
}

let dpr: number = 0
let deRect: mixed = null
let screenWidth: number = 0
let screenHeight: number = 0

const info: {
  dpr: number,
  scale: number,
  rem: number,
  deviceWidth: number,
  deviceHeight: number
} = {
  dpr,
  scale: 0,
  rem: 0,
  deviceWidth: 0,
  deviceHeight: 0
}

/**
 * set root font-size for rem units. If already been set, just skip this.
 */
function setRootFont (width: number): void {
  const doc = window.document
  const rem = width / 10
  if (!doc.documentElement) { return }
  const rootFontSize = doc.documentElement.style.fontSize
  if (!rootFontSize) {
    doc.documentElement.style.fontSize = rem + 'px'
    info.rem = rem
  }
}

function setMetaViewport (width: number): void {
  if (!wxViewportMeta) {
    wxViewportMeta = document.createElement('meta')
    wxViewportMeta.setAttribute('name', 'weex-viewport')
  }
  else {
    const metaWidth = parseInt(wxViewportMeta.getAttribute('content'))
    if (metaWidth === width) {
      return
    }
  }
  wxViewportMeta.setAttribute('content', width + '')
}

/**
 * export viewport info.
 */
export function init (viewportWidth: number = width): ?{
  dpr: number,
  scale: number,
  rem: number,
  deviceWidth: number,
  deviceHeight: number
} {
  if (!isInited) {
    isInited = true

    const doc = window.document
    if (!doc) {
      console.error('[vue-render] window.document is undfined.')
      return
    }
    if (!doc.documentElement) {
      console.error('[vue-render] document.documentElement is undfined.')
      return
    }

    dpr = info.dpr = window.devicePixelRatio
    deRect = doc.documentElement.getBoundingClientRect()
    screenWidth = deRect.width
    screenHeight = deRect.height

    // set root font for rem.
    setRootFont(screenWidth)
    setMetaViewport(viewportWidth)

    /**
     * why not to use window.screen.width to get screenWidth ? Because in some
     * old webkit browser on android system it get the device pixel width, which
     * is the screenWidth multiply by the device pixel ratio.
     * e.g. ip6 -> get 375 for virtual screen width.
     */
    const scale = screenWidth / viewportWidth
    /**
     * 1. if set initial/maximum/mimimum-scale some how the page will have a bounce
     * effect when user drag the page towards horizontal axis.
     * 2. Due to compatibility reasons, not to use viewport meta anymore. Just bring
     * a parameter scale into the style value processing.
     */

    // const contents = [
    //   `width=${viewportWidth}`,
    //   `initial-scale=${scale}`,
    //   `maximum-scale=${scale}`,
    //   `minimum-scale=${scale}`,
    //   `user-scalable=no`
    // ]

    // let meta = doc.querySelector('meta[name="viewport"]')
    // if (!meta) {
    //   meta = doc.createElement('meta')
    //   meta.setAttribute('name', 'viewport')
    //   document.querySelector('head').appendChild(meta)
    // }
    // meta.setAttribute('content', contents.join(','))

    extend(info, {
      scale,
      deviceWidth: screenWidth * dpr,
      deviceHeight: screenHeight * dpr
    })
  }

  return info
}

/**
 * reset viewport width and scale.
 * @return new scale.
 */
export function resetViewport (viewportWidth: number): number {
  setMetaViewport(viewportWidth)
  const newScale = screenWidth / viewportWidth
  info.scale = newScale
  return newScale
}

export function getViewportInfo (): {
  dpr: number,
  scale: number,
  rem: number,
  deviceWidth: number,
  deviceHeight: number
} {
  return info
}
