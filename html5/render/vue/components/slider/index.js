import { validateStyles } from '../../validator'
import { throttle, bind } from '../../utils'
import indicator from './indicator'

export default {
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

  components: { indicator },

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

    slideTo (index) {
      let newIndex = (index | 0) // % this.frameCount
      // let newIndex = (index | 0) % this.frameCount // scroll to left
      newIndex = Math.max(newIndex, 0)
      newIndex = Math.min(newIndex, this.frameCount - 1)

      const offset = -newIndex * this.wrapperWidth
      const inner = this.$refs.inner

      if (inner) {
        inner.style.transform = `translate3d(${offset}px, 0, 0)`
      }
      this.currentIndex = newIndex
    },

    next () {
      this.slide(this.currentIndex + 1)
    },

    prev () {
      this.slide(this.currentIndex - 1)
    },

    handleTouchStart (event) {
      event.preventDefault()
      // console.log('touch start', event)
      const touch = event.changedTouches[0]
      // console.log('touch start', event.target, event.target.pageY)
      // console.log('touches', touch)
      this._touchParams = {
        originalTransform: this.$refs.inner.style.transform,
        startTouchEvent: touch,
        startX: touch.pageX,
        startY: touch.pageY,
        timeStamp: event.timeStamp
      }
    },

    handleTouchMove (event) {
      event.preventDefault()
      // console.log('touch move')
      if (this._touchParams) {
        const inner = this.$refs.inner
        const { startX } = this._touchParams
        const touch = event.changedTouches[0]
        const offsetX = touch.pageX - startX
        // console.log('offsetX', offsetX, 'startX', startX, 'pageX', touch.pageX)
        this._touchParams.offsetX = offsetX

        if (inner && offsetX) {
          // console.log('transform', `${offsetX - this.currentIndex * this.wrapperWidth}`)
          inner.style.transform = `translate3d(${offsetX - this.currentIndex * this.wrapperWidth}px, 0, 0)`
        }
      }
    },

    handleTouchEnd (event) {
      event.preventDefault()
      // console.log('touch end')
      const inner = this.$refs.inner
      if (this._touchParams) {
        const { offsetX } = this._touchParams
        // console.log('touch pageX:', touch.pageX, ', offsetX:', offsetX)
        if (inner) {
          inner.style.transition = `transform .2s ease-in-out`

          const reset = Math.abs(offsetX / this.wrapperWidth) < 0.2
          const direction = offsetX > 0 ? 1 : -1
          const newIndex = reset ? this.currentIndex : (this.currentIndex - direction)

          // console.log('reset:', reset, ', newIndex:', newIndex)
          this.slideTo(newIndex)
        }
      }
      delete this._touchParams
    }
  },

  created () {
    this.$nextTick(() => {
      this.updateLayout()
    })
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

    return createElement('nav', {
      ref: 'wrapper',
      attrs: { 'weex-type': 'slider' },
      staticClass: 'weex-slider weex-slider-wrapper',
      on: {
        touchstart: this.handleTouchStart,
        touchmove: throttle(bind(this.handleTouchMove, this), 25),
        touchend: this.handleTouchEnd
      }
    }, [createElement('ul', {
      ref: 'inner',
      staticClass: 'weex-slider-inner'
    }, innerElements), createElement(indicator, {
      attrs: {
        count: this.frameCount,
        active: this.currentIndex
      }
    })])
  }
}
