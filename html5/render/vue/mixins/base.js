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
  created () {
    this._prerender()
  },
  mounted () {
    watchAppear(this)
    // watchLazyload(this)
  },

  beforeUpdate () {
    this._prerender()
  },

  methods: {
    _getTopContext () {
      // let ctx = this
      // let vnode = ctx.$vnode
      // while (vnode) {
      //   ctx = vnode.context
      //   vnode = ctx.$vnode
      // }
      // return ctx
      return this.$options._parentVnode.context
    },

    _getScopeId () {
      const ctx = this._getTopContext()
      return ctx.$options._scopeId
    },

    _getParentScroller () {
      return _getParentScroller(this.$vnode)
    },

    _createEventMap (extras = []) {
      const eventMap = {}
      supportedEvents.concat(extras).forEach(name => {
        eventMap[name] = event => this.$emit(name, event)
      })
      return eventMap
    },

    _fireLazyload () {
      const scroller = this._getParentScroller()
      fireLazyload(scroller && scroller.$el || document.body)
    },

    _prerender () {
      this._mergeStyles()
      // process prerender hooks for components' own treatment.
      this.beforeRender && this.beforeRender()
    }
  }
}
