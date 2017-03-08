/**
 * viewport priority:
 *
 * 1. meta viewport (developer custom)
 * 2. setViewport(config) := config.width (private code) @deprecated
 * 3. process.env.VIEWPORT_WIDTH (buid time)
 *
 */
let viewportWidth = process.env.VIEWPORT_WIDTH

const wxViewportMeta = document.querySelector('meta[name="weex-viewport"]')
const metaWidth = wxViewportMeta && parseInt(wxViewportMeta.getAttribute('content'))
if (metaWidth && !isNaN(metaWidth) && metaWidth > 0) { viewportWidth = metaWidth }

export function setViewport (config = {}) {
  const doc = window.document

  if (doc) {
    // const viewportWidth = parseViewportWidth(config)

    // set root font-size
    // doc.documentElement.style.fontSize = viewportWidth / 10 + 'px'

    /**
     * why not to use window.screen.width to get screenWidth ? Because in some
     * old webkit browser on android system it get the device pixel width, which
     * is the screenWidth multiply by the device pixel ratio.
     */
    const deRect = document.documentElement.getBoundingClientRect()
    const screenWidth = deRect.width
    const screenHeight = deRect.height
    const scale = screenWidth / viewportWidth
    const contents = [
      `width=${viewportWidth}`,
      `initial-scale=${scale}`,
      `maximum-scale=${scale}`,
      `minimum-scale=${scale}`,
      `user-scalable=no`
    ]

    let meta = doc.querySelector('meta[name="viewport"]')
    if (!meta) {
      meta = doc.createElement('meta')
      meta.setAttribute('name', 'viewport')
      document.querySelector('head').appendChild(meta)
    }
    meta.setAttribute('content', contents.join(','))

    const dpr = window.devicePixelRatio

    return {
      scale,
      deviceWidth: screenWidth * dpr,
      deviceHeight: screenHeight * dpr
    }
  }
}
