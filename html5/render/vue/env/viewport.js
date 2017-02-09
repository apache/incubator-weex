const DEFAULT_VIEWPORT_WIDTH = 750

function parseViewportWidth (config) {
  let width = DEFAULT_VIEWPORT_WIDTH
  if (config && config.width) {
    width = Number(config.width) || config.width
  }
  return width
}

export function setViewport (config = {}) {
  const doc = window.document

  if (doc) {
    const viewportWidth = parseViewportWidth(config)

    // set root font-size
    doc.documentElement.style.fontSize = viewportWidth / 10 + 'px'

    const screenWidth = window.screen.width
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
  }
}
