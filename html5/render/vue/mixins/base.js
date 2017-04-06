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
    if (process.env.NODE_ENV === 'development') {
      tagBeforeCreate()
    }
  },

  mounted () {
    if (!weex._root) {
      weex._root = this.$root.$el
      weex._root.classList.add('weex-root')
    }
    watchAppear(this)
    if (process.env.NODE_ENV === 'development') {
      tagMounted()
    }
  },

  beforeUpdate () {
    if (process.env.NODE_ENV === 'development') {
      tagBeforeUpdate()
    }
  },

  updated () {
    if (process.env.NODE_ENV === 'development') {
      tagUpdated()
    }
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

    _getScopeIds () {
      const arr = []
      let ctx = this
      let scopeId
      while (ctx) {
        scopeId = ctx.$options._scopeId
        scopeId && arr.push(scopeId)
        ctx = ctx.$options.parent
      }
      return arr
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
    },

    _trimTextNodeChildren (children) {
      if (Array.isArray(children)) {
        return children.filter(vnode => !!vnode.tag)
      }
      return children
    }
  }
}
