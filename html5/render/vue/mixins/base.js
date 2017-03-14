import { getThrottleLazyload, watchAppear } from '../utils'
import { tagBeforeCreate, tagMounted, tagBeforeUpdate, tagUpdated, tagBegin, tagEnd } from '../utils/perf'

const supportedEvents = [
  'click', 'longpress', 'appear', 'disappear'
  // 'touchstart', 'touchmove', 'touchend'
]

const scrollableTypes = ['scroller', 'list']

let lazyloadWatched = false
function watchLazyload () {
  lazyloadWatched = true
  ; [
    'scroll'
    // 'transitionend',
    // 'webkitTransitionEnd',
    // 'animationend',
    // 'webkitAnimationEnd',
    // 'resize'
  ].forEach(evt => {
    window.addEventListener(evt, getThrottleLazyload(25, document.body))
  })
}

export default {
  beforeCreate () {
    if (!lazyloadWatched) {
      watchLazyload()
    }
    tagBeforeCreate()
  },

  mounted () {
    if (!weex._root) {
      weex._root = this.$root.$el
      weex._root.classList.add('weex-root')
      Object.freeze(weex)
    }
    watchAppear(this)
    tagMounted()
  },

  beforeUpdate () {
    tagBeforeUpdate()
  },

  updated () {
    tagUpdated()
  },

  methods: {
    _getTopContext () {
      let ctx = this
      let vnode = ctx.$options._parentVnode
      while (vnode) {
        ctx = vnode.context
        vnode = ctx.$options._parentVnode
      }
      return ctx
    },

    _getScopeId () {
      // return closest scopeId.
      let scopeId = this.$options._scopeId
      let ctx = this
      while (!scopeId) {
        ctx = ctx.$options.parent
        if (!ctx) return null
        scopeId = ctx.$options._scopeId
      }
      return scopeId
    },

    _getParentScroller () {
      let parent = this
      while (parent && scrollableTypes.indexOf(parent.$options._componentTag) <= -1) {
        parent = parent.$options.parent
      }
      return parent
    },

    _createEventMap (extras = []) {
      const eventMap = {}
      supportedEvents.concat(extras).forEach(name => {
        eventMap[name] = event => this.$emit(name, event)
      })
      return eventMap
    },

    _fireLazyload (el) {
      if (process.env.NODE_ENV === 'development') {
        tagBegin('base._fireLazyload')
      }
      getThrottleLazyload(16)()
      if (process.env.NODE_ENV === 'development') {
        tagEnd('base._fireLazyload')
      }
    }
  }
}
