import { watchAppear, fireLazyload } from '../utils'

const supportedEvents = [
  'click', 'longpress', 'appear', 'disappear'
  // 'touchstart', 'touchmove', 'touchend'
]

const scrollableTypes = ['scroller', 'list']

// function watchLazyload (context) {
//   [
//     'scroll',
//     'transitionend',
//     'webkitTransitionEnd',
//     'animationend',
//     'webkitAnimationEnd',
//     'resize'
//   ].forEach(evt => {
//     fireLazyload(document.body)
//   })
// }

function _getParentScroller (vnode) {
  if (!vnode) return null
  if (scrollableTypes.indexOf(vnode.weexType) > -1) {
    return vnode
  }
  return _getParentScroller(vnode.$parent)
}

export default {
  mounted () {
    watchAppear(this)
    // watchLazyload(this)
  },

  methods: {
    getTopContext () {
      let ctx = this
      let vnode = ctx.$vnode
      while (vnode) {
        ctx = vnode.context
        vnode = ctx.$vnode
      }
      return ctx
    },

    getScopeId () {
      const ctx = this.getTopContext()
      return ctx.$options._scopeId
    },

    getParentScroller () {
      return _getParentScroller(this.$vnode)
    },

    createEventMap (extras = []) {
      const eventMap = {}
      supportedEvents.concat(extras).forEach(name => {
        eventMap[name] = event => this.$emit(name, event)
      })
      return eventMap
    },

    fireLazyload () {
      const scroller = this.getParentScroller()
      fireLazyload(scroller && scroller.$el || document.body)
    }
  }
}
