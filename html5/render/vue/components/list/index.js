import { validateStyles } from '../../validator'
import { debounce, throttle, bind } from '../../utils'
import refresh from './refresh'
import loading from './loading'
import rectMixin from '../../mixins/rect'
import eventMixin from '../../mixins/event'
import listMixin from './listMixin'

export default {
  mixins: [rectMixin, eventMixin, listMixin],
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
    },
    createLoading () {
      //
    },
    createRefresh () {
      //
    },
    createChildren (createElement) {
      const slots = this.$slots.default || []
      const cells = slots.filter(vnode => {
        // console.log(vnode.tag)
        if (!vnode.tag || !vnode.componentOptions) return false
        const tagName = vnode.componentOptions.tag
        if (tagName === 'loading') {
          this._loading = createElement(loading, {
            on: {
              loading: () => this.$emit('loading', this.createCustomEvent('loading'))
            }
          })
          return false
        }
        if (tagName === 'refresh') {
          this._refresh = createElement(refresh, {
            on: {
              refresh: () => this.$emit('refresh', this.createCustomEvent('refresh'))
            }
          })
          return false
        }
        return true
      })
      return [
        this._refresh,
        createElement('div', {
          ref: 'inner',
          staticClass: 'weex-list-inner'
        }, cells),
        this._loading
      ]
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
        scroll: debounce(bind(this.handleScroll, this), 100),
        touchstart: this.handleTouchStart,
        touchmove: throttle(bind(this.handleTouchMove, this), 25),
        touchend: this.handleTouchEnd
      }
    }, this.createChildren(createElement))
  }
}
