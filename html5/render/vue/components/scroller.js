import { validateStyles } from '../validator'
import { debounce, bind } from '../utils'
import rectMixin from '../mixins/rect'

export default {
  mixins: [rectMixin],
  props: {
    scrollDirection: {
      type: [String],
      default: 'vertical',
      validator (value) {
        return ['horizontal', 'vertical'].indexOf(value) !== -1
      }
    },
    loadmoreoffset: {
      type: [String, Number],
      default: 0
    }
  },

  computed: {
    wrapperClass () {
      const classArray = ['weex-scroller', 'weex-scroller-wrapper']
      if (this.scrollDirection === 'horizontal') {
        classArray.push('weex-scroller-horizontal')
      }
      return classArray.join(' ')
    }
  },

  methods: {
    handleScroll (event) {
      if (this.reachBottom()) {
        this.$emit('loadmore', event)
      }
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('scroller', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    return createElement('main', {
      ref: 'wrapper',
      attrs: { 'weex-type': 'scroller' },
      staticClass: this.wrapperClass,
      on: {
        scroll: debounce(bind(this.handleScroll, this), 100)
      }
    }, [
      createElement('mark', { ref: 'topMark', staticClass: 'weex-scroller-top-mark' }),
      createElement('div', {
        ref: 'inner',
        staticClass: 'weex-scroller-inner'
      }, this.$slots.default),
      createElement('mark', { ref: 'bottomMark', staticClass: 'weex-scroller-bottom-mark' })
    ])
  }
}
