import '../../shared/arrayFrom'
import '../../shared/objectAssign'
import '../../shared/objectSetPrototypeOf'

// TODO: Promise polyfill

const DEFAULT_VIEWPORT_WIDTH = 750

export function setViewport (configs = {}) {
  const doc = window.document
  const contents = []
  for (const key in configs) {
    contents.push(`${key}=${configs[key]}`)
  }

  if (doc) {
    let meta = doc.querySelector('meta[name="viewport"]')
    if (!meta) {
      meta = doc.createElement('meta')
      meta.setAttribute('name', 'viewport')
      document.querySelector('head').appendChild(meta)
    }
    meta.setAttribute('content', contents.join(','))
  }
}

export function updateMeta () {
  // const dpr = window.devicePixelRatio
  // const docWidth = document.documentElement.getBoundingClientRect().width
  const screenWidth = window.screen.width
  const scale = screenWidth / DEFAULT_VIEWPORT_WIDTH

  setViewport({
    width: DEFAULT_VIEWPORT_WIDTH,
    'initial-scale': scale,
    'maximum-scale': scale,
    'minimum-scale': scale,
    'user-scalable': 'no'
  })
}
