import { validateStyles } from '../validator'

export default {
  props: {
    'show-scrollbar': [String, Boolean],
    'scroll-direction': {
      type: [String],
      default: 'vertical'
    },
    loadmoreoffset: {
      type: [String, Number],
      default: 0
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('scroller', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    return createElement('main', {
      attrs: {
        'weex-type': 'scroller'
      },
      staticClass: 'weex-scroller weex-scroller-wrapper'
    }, [createElement('div', {
      staticClass: 'weex-scroller-inner'
    }, this.$slots.default)])
  }
}
