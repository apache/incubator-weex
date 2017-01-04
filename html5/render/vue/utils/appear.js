import { throttle } from './func'

const watchedComponents = []
let listened = false
let scrollY = 0

function needWatch (vnode) {
  const events = vnode.data.event
  if (events
      && (events.indexOf('appear') !== -1
        || events.indexOf('disappear') !== -1)) {
    return true
  }
  return false
}

export function watchIfNeeded (vnode) {
  if (needWatch(vnode)) {
    watchedComponents.push(vnode)
    if (!listened) {
      listened = true
      const handler = throttle(onScroll, 100)
      window.addEventListener('scroll', handler, false)
    }
  }
}

export function isComponentInWindow (vnode) {
  const rect = vnode.elm.getBoundingClientRect()
  return rect.right > 0 && rect.left < window.innerWidth &&
         rect.bottom > 0 && rect.top < window.innerHeight
}

export function hasIntersection (rect, ctRect) {
  return (rect.left < ctRect.right && rect.right > ctRect.left)
    && (rect.top < ctRect.bottom && rect.bottom > ctRect.top)
}

export function getParentScroller (vnode) {
  if (!vnode) return null
  if (vnode.weexType === 'scroller' || vnode.weexType === 'list') {
    return vnode
  }
  return getParentScroller(vnode.$parent)
}

export function isComponentAppear (vnode) {
  const parentScroller = getParentScroller(vnode)
  if (!parentScroller) {
    return isComponentInWindow(vnode)
  }
  return isComponentInWindow(vnode)
    && hasIntersection(
      vnode.elm.getBoundingClientRect(),
      parentScroller.$el.getBoundingClientRect())
}

function onScroll (e) {
  let direction
  // NOTE: this condition strongly relies on the scroller's implementation.
  if (e.originalType === 'scrolling') {
    direction = e.direction
  }
  else {
    // NOTE: only VERTICAL window scroll can be detected.
    const y = window.scrollY
    direction = y >= scrollY ? 'up' : 'down'
    scrollY = y
  }
  const len = watchedComponents.length
  for (let i = 0; i < len; i++) {
    const vnode = watchedComponents[i]
    const appear = isComponentAppear(vnode)
    if (appear) {
      vnode.elm && vnode.elm.dispatchEvent('appear', { direction })
    }
    else if (!appear) {
      vnode.elm && vnode.elm.dispatchEvent('disappear', { direction })
    }
  }
}
