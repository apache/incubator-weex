// import { validateStyles } from '../../validator'
import { extractComponentStyle, createEventMap } from '../../core'
import { throttle, bind, extend, fireLazyload } from '../../utils'
import indicator from './indicator'
import slideMixin from './slideMixin'

const _css = `
.weex-slider-wrapper {
  overflow: hidden;
}

.weex-slider-inner {
  position: absolute;
  height: 100%;
  top: 0;
  left: 0;
}

.weex-slider-cell {
  display: block;
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  margin: 0;
  padding: 0;
  height: 100%;
  overflow: hidden;
}
`

export default {
  mixins: [slideMixin],
  props: {
    'auto-play': {
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

  created () {
    this.weexType = 'slider'
    this.currentIndex = 0
    this.innerOffset = 0
    this._indicator = null
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
        attrs: { 'weex-type': 'slider' },
        staticClass: 'weex-slider weex-slider-wrapper weex-ct',
        on: extend(createEventMap(this, ['scroll', 'scrollstart', 'scrollend']), {
          touchstart: this.handleTouchStart,
          touchmove: throttle(bind(this.handleTouchMove, this), 25),
          touchend: this.handleTouchEnd
        }),
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
  },
  // export default css styles for this component.
  _css
}
