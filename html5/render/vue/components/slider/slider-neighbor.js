// import { validateStyles } from '../../validator'
import './slider-neighbor.css'
import { extractComponentStyle, createEventMap } from '../../core'
import { throttle, bind, extend, fireLazyload } from '../../utils'
import indicator from './indicator'
import slideMixin from './slideMixin'

const DEFAULT_NEIGHBOR_SPACE = 20
const DEFAULT_NEIGHBOR_ALPHA = 0.6
const DEFAULT_NEIGHBOR_SCALE = 0.8

let id = 0
export default {
  mixins: [slideMixin],
  props: {
    autoPlay: {
      type: [String, Boolean],
      default: false
    },
    interval: {
      type: [String, Number],
      default: 3000
    },
    infinite: {
      type: [String, Boolean],
      default: true
    },
    neighborSpace: {
      type: [String, Number],
      validator: function (val) {
        val = parseFloat(val)
        return !isNaN(val) && val > 0
      },
      default: DEFAULT_NEIGHBOR_SPACE
    },
    neighborAlpha: {
      type: [String, Number],
      validator: function (val) {
        val = parseFloat(val)
        return !isNaN(val) && val >= 0 && val <= 1
      },
      default: DEFAULT_NEIGHBOR_ALPHA
    },
    neighborScale: {
      type: [String, Number],
      validator: function (val) {
        val = parseFloat(val)
        return !isNaN(val) && val >= 0 && val <= 1
      },
      default: DEFAULT_NEIGHBOR_SCALE
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
    },

    formatChildren (createElement) {
      const children = this.$slots.default || []
      let indicatorVnode
      const cells = children.filter(vnode => {
        if (!vnode.tag) return false
        if (vnode.componentOptions && vnode.componentOptions.tag === 'indicator') {
          indicatorVnode = vnode
          return false
        }
        return true
      }).map(vnode => {
        return createElement('li', {
          ref: 'cells',
          staticClass: 'weex-slider-cell'
        }, [vnode])
      })
      if (indicatorVnode) {
        indicatorVnode.data.attrs = indicatorVnode.data.attrs || {}
        indicatorVnode.data.attrs.count = cells.length
        indicatorVnode.data.attrs.active = this.currentIndex
        this._indicator = createElement(indicator, indicatorVnode.data)
      }
      return cells
    }
  },

  beforeCreate () {
    this.isNeighbor = true
  },

  created () {
    this.weexType = 'slider-neighbor'
    this.currentIndex = 0
    this.innerOffset = 0
    this._indicator = null
    this.id = id++
    this.$nextTick(() => {
      this.updateLayout()
    })
  },

  beforeUpdate () {
    this.updateLayout()
    this.reorder()
  },

  updated () {
    fireLazyload(this.$el, true)
  },

  mounted () {
    if (this.autoPlay && this.autoPlay !== 'false') {
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

    this.reorder()
    fireLazyload(this.$el, true)
  },

  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('slider', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    this._cells = this.formatChildren(createElement)
    this.frameCount = this._cells.length

    return createElement(
      'nav',
      {
        ref: 'wrapper',
        attrs: { 'weex-type': 'slider-neighbor' },
        on: extend(createEventMap(this, ['scroll', 'scrollstart', 'scrollend']), {
          touchstart: this.handleTouchStart,
          touchmove: throttle(bind(this.handleTouchMove, this), 25),
          touchend: this.handleTouchEnd
        }),
        staticClass: 'weex-slider weex-slider-wrapper weex-ct',
        staticStyle: extractComponentStyle(this)
      },
      [
        createElement('ul', {
          ref: 'inner',
          staticClass: 'weex-slider-inner weex-ct'
        }, this._cells),
        this._indicator
      ]
    )
  }
}
