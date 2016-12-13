import { validateStyles } from '../../validator'
import { throttle, bind } from '../../utils'
import indicator from './indicator'
import eventMixin from '../../mixins/event'
import slideMixin from './slideMixin'

export default {
  mixins: [eventMixin, slideMixin],
  components: { indicator },
  props: {
    'auto-play': {
      type: [String, Boolean],
      default: false
    },
    interval: {
      type: [String, Number],
      default: 3000
    }
  },

  data () {
    return {
      currentIndex: 0,
      frameCount: 0
    }
  },

  methods: {
    computeWrapperSize () {
      const wrapper = this.$refs.wrapper
      if (wrapper) {
        const rect = wrapper.getBoundingClientRect()
        this.wrapperWidth = rect.width
        this.wrapperHeight = rect.height
      }
    },

    updateLayout () {
      this.computeWrapperSize()
      const inner = this.$refs.inner
      if (inner) {
        inner.style.width = this.wrapperWidth * this.frameCount + 'px'
      }
    }
  },

  created () {
    this.$nextTick(() => {
      this.updateLayout()
    })
  },

  mounted () {
    if (this.autoPlay) {
      const interval = Number(this.interval)
      this._lastSlideTime = Date.now()

      const autoPlayFn = bind(function () {
        clearTimeout(this._autoPlayTimer)
        const now = Date.now()
        let nextTick = interval - now + this._lastSlideTime
        nextTick = nextTick > 100 ? nextTick : interval

        this.next()
        this._lastSlideTime = now
        this._autoPlayTimer = setTimeout(autoPlayFn, nextTick)
      }, this)

      this._autoPlayTimer = setTimeout(autoPlayFn, interval)
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('slider', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    const children = this.$slots.default || []
    const innerElements = children.map(vnode => createElement('li', {
      staticClass: 'weex-slider-cell'
    }, [vnode]))
    this.frameCount = innerElements.length

    return createElement(
      'nav',
      {
        ref: 'wrapper',
        attrs: { 'weex-type': 'slider' },
        staticClass: 'weex-slider weex-slider-wrapper',
        on: {
          touchstart: this.handleTouchStart,
          touchmove: throttle(bind(this.handleTouchMove, this), 25),
          touchend: this.handleTouchEnd
        }
      },
      [
        createElement('ul', {
          ref: 'inner',
          staticClass: 'weex-slider-inner'
        }, innerElements),
        createElement(indicator, {
          attrs: {
            count: this.frameCount,
            active: this.currentIndex
          }
        })
      ]
    )
  }
}
