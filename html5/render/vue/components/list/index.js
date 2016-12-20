import { validateStyles } from '../../validator'
import { debounce, throttle, bind, createMixin } from '../../utils'
import refresh from './refresh'
import loading from './loading'
import * as rectMethods from '../../methods/rect'
import * as eventMethods from '../../methods/event'
import listMixin from './listMixin'

export default {
  mixins: [createMixin(rectMethods, eventMethods), listMixin],
  props: {
    loadmoreoffset: {
      type: [String, Number],
      default: 0
    }
  },

  computed: {
    wrapperClass () {
      const classArray = ['weex-list', 'weex-list-wrapper']
      this._refresh && classArray.push('with-refresh')
      this._loading && classArray.push('with-loading')
      return classArray.join(' ')
    }
  },

  methods: {
    updateLayout () {
      this.computeWrapperSize()
      if (this._cells && this._cells.length) {
        this._cells.forEach(vnode => {
          vnode._visible = this.isCellVisible(vnode.elm)
        })
      }
    },
    isCellVisible (elem) {
      if (!this.wrapperHeight) {
        this.computeWrapperSize()
      }
      const wrapper = this.$refs.wrapper
      return wrapper.scrollTop <= elem.offsetTop
        && elem.offsetTop < wrapper.scrollTop + this.wrapperHeight
    },

    handleScroll (event) {
      this._cells.forEach((vnode, index) => {
        const visible = this.isCellVisible(vnode.elm)
        if (visible !== vnode._visible) {
          const type = visible ? 'appear' : 'disappear'
          vnode._visible = visible

          // TODO: dispatch CustomEvent
          vnode.elm.dispatchEvent(new Event(type), {})
        }
      })
      if (this.reachBottom()) {
        this.$emit('loadmore', this.createCustomEvent('loadmore'))
      }
    },

    createChildren (createElement) {
      const slots = this.$slots.default || []
      this._cells = slots.filter(vnode => {
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
        }, this._cells),
        this._loading
      ]
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('list', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    this.$nextTick(() => {
      this.updateLayout()
    })

    return createElement('main', {
      ref: 'wrapper',
      attrs: { 'weex-type': 'list' },
      staticClass: this.wrapperClass,
      on: {
        scroll: debounce(bind(this.handleScroll, this), 30),
        touchstart: this.handleTouchStart,
        touchmove: throttle(bind(this.handleTouchMove, this), 25),
        touchend: this.handleTouchEnd
      }
    }, this.createChildren(createElement))
  }
}
