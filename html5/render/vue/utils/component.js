import { throttle, extend } from './func'
import { createEvent } from './event'

export function getParentScroller (vnode) {
  if (!vnode) return null
  if (vnode.weexType === 'scroller' || vnode.weexType === 'list') {
    return vnode
  }
  return getParentScroller(vnode.$parent)
}

export function hasIntersection (rect, ctRect) {
  return (rect.left < ctRect.right && rect.right > ctRect.left)
    && (rect.top < ctRect.bottom && rect.bottom > ctRect.top)
}

/**
 * [isElementVisible description]
 * @param  {HTMLElement}  el    a dom element.
 * @param  {HTMLElement}  container  optional, the container of this el.
 */
export function isElementVisible (el, container) {
  const bodyRect = {
    top: 0,
    left: 0,
    bottom: window.innerHeight,
    right: window.innerWidth
  }
  const ctRect = (container === document.body)
    ? bodyRect : container
    ? container.getBoundingClientRect() : bodyRect
  return hasIntersection(
    el.getBoundingClientRect(),
    ctRect)
}

export function isComponentVisible (component) {
  if (component.$el) {
    const scroller = getParentScroller(component)
    if (scroller && scroller.$el) {
      return hasIntersection(
        component.$el.getBoundingClientRect(),
        scroller.$el.getBoundingClientRect()
      )
    }
    else {
      return isElementVisible(component.$el)
    }
  }
  return false
}

// TODO: improve the efficiency
export function watchAppear (context) {
  if (!context) return null

  context.$nextTick(() => {
    if ((context.$options && context.$options._parentListeners)
      || (context.$vnode && context.$vnode.data && context.$vnode.data.on)) {
      const on = extend({}, context.$options._parentListeners, context.$vnode.data.on)
      if (on.appear || on.disappear) {
        const scroller = getParentScroller(context)
        let isWindow = false
        let container = window
        if (scroller && scroller.$el) {
          container = scroller.$el
        }
        else {
          isWindow = true
        }
        let lastScrollTop = container.scrollTop || window.pageYOffset

        context._visible = isElementVisible(context.$el, isWindow ? document.body : container)
        if (context._visible && on.appear) {
          if (on.appear.fn) {
            on.appear = on.appear.fn
          }
          on.appear(createEvent(context.$el, 'appear', { direction: null }))
        }
        const handler = throttle(event => {
          const visible = isElementVisible(context.$el, isWindow ? document.body : container)
          let listener = null
          let type = null
          if (visible !== context._visible) {
            context._visible = visible
            if (visible) {
              listener = on.appear
              type = 'appear'
            }
            else {
              listener = on.disappear
              type = 'disappear'
            }
            if (listener && listener.fn) {
              listener = listener.fn
            }
            const scrollTop = container.scrollTop || window.pageYOffset
            listener && listener(createEvent(context.$el, type, {
              direction: scrollTop < lastScrollTop ? 'down'
                : scrollTop > lastScrollTop ? 'up' : null
            }))
            lastScrollTop = scrollTop
          }
        }, 25, true)

        container.addEventListener('scroll', handler, false)
      }
    }
  })
}
