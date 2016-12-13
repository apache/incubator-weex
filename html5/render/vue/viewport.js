const DEFAULT_VIEWPORT_WIDTH = 750

export function setViewport (configs = {}) {
  const doc = window.document

  if (doc) {
    const screenWidth = window.screen.width
    const scale = screenWidth / DEFAULT_VIEWPORT_WIDTH

    const contents = [
      `width=${DEFAULT_VIEWPORT_WIDTH}`,
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
