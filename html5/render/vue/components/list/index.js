import { validateStyles } from '../../validator'
import { debounce, bind } from '../../utils'
import rectMixin from '../../mixins/rect'
import eventMixin from '../../mixins/event'

export default {
  mixins: [rectMixin, eventMixin],
  props: {
    loadmoreoffset: {
      type: [String, Number],
      default: 0
    }
  },

  methods: {
    handleScroll (event) {
      if (this.reachBottom()) {
        this.$emit('loadmore', this.createCustomEvent('loadmore'))
      }
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('list', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    return createElement('main', {
      ref: 'wrapper',
      attrs: { 'weex-type': 'list' },
      staticClass: 'weex-list weex-list-wrapper',
      on: {
        scroll: debounce(bind(this.handleScroll, this), 100)
      }
    }, [
      createElement('mark', { ref: 'topMark', staticClass: 'weex-list-top-mark' }),
      createElement('div', {
        ref: 'inner',
        staticClass: 'weex-list-inner'
      }, this.$slots.default),
      createElement('mark', { ref: 'bottomMark', staticClass: 'weex-list-bottom-mark' })
    ])
  }
}
