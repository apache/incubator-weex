import { getThrottleLazyload, throttle } from '../utils'

let throttleScroll
function getThrottledScroll (context) {
  if (!throttleScroll) {
    const wrapper = context.$refs.wrapper
    const inner = context.$refs.inner
    let preOffset = (context.scrollDirection === 'horizontal'
        ? wrapper.scrollLeft
        : wrapper.scrollTop)
      || 0
    throttleScroll = throttle(function (evt) {
      const offset = context.scrollDirection === 'horizontal'
        ? wrapper.scrollLeft
        : wrapper.scrollTop
      const indent = parseInt(context.offsetAccuracy)
      function triggerScroll () {
        const rect = inner.getBoundingClientRect()
        evt.contentSize = { width: rect.width, height: rect.height }
        evt.contentOffset = {
          x: wrapper.scrollLeft,
          /**
           * positive direciton for y-axis is down.
           * so should use negative operation on scrollTop.
           *
           *  (0,0)---------------> x
           *       |
           *       |
           *       |
           *       |
           *       v y
           *
           */
          y: -wrapper.scrollTop
        }
        context.$emit('scroll', evt)
      }
      if (indent
        && !isNaN(indent)
        && indent > 0
        && Math.abs(offset - preOffset) >= indent) {
        triggerScroll()
        preOffset = offset
      }
      else if (!indent || isNaN(indent) || indent <= 0) {
        triggerScroll()
      }
    }, 16, true)
  }
  return throttleScroll
}

export default {
  props: {
    offsetAccuracy: [Number, String]
  },
  methods: {
    updateLayout () {
      const wrapper = this.$refs.wrapper
      if (wrapper) {
        const rect = wrapper.getBoundingClientRect()
        this.wrapperWidth = rect.width
        this.wrapperHeight = rect.height
      }
    },

    handleScroll (event) {
      getThrottleLazyload(25, this.$el, 'scroll')()
      getThrottledScroll(this)(event)
      if (this.reachBottom()) {
        this.$emit('loadmore', event)
      }
    },

    reachTop () {
      const wrapper = this.$refs.wrapper
      return (!!wrapper) && (wrapper.scrollTop <= 0)
    },

    reachBottom () {
      const wrapper = this.$refs.wrapper
      const inner = this.$refs.inner
      const offset = Number(this.loadmoreoffset) || 0

      if (wrapper && inner) {
        const innerHeight = inner.getBoundingClientRect().height
        const wrapperHeight = wrapper.getBoundingClientRect().height
        return wrapper.scrollTop >= innerHeight - wrapperHeight - offset
      }
      return false
    },

    handleTouchStart (event) {
      // event.preventDefault()
      event.stopPropagation()
      if (this._loading || this._refresh) {
        const touch = event.changedTouches[0]
        this._touchParams = {
          reachTop: this.reachTop(),
          reachBottom: this.reachBottom(),
          startTouchEvent: touch,
          startX: touch.pageX,
          startY: touch.pageY,
          timeStamp: event.timeStamp
        }
      }
    },

    handleTouchMove (event) {
      // event.preventDefault()
      event.stopPropagation()
      if (this._touchParams) {
        const inner = this.$refs.inner
        const { startY, reachTop, reachBottom } = this._touchParams
        if (inner) {
          const touch = event.changedTouches[0]
          const offsetY = touch.pageY - startY
          this._touchParams.offsetY = offsetY
          if (reachTop && this._refresh) {
            this._refresh.pullingDown(offsetY)
          }
          else if (reachBottom && this._loading) {
            this._loading.pullingUp(-offsetY)
          }
        }
      }
    },

    handleTouchEnd (event) {
      // event.preventDefault()
      event.stopPropagation()
      if (this._touchParams) {
        const inner = this.$refs.inner
        const { reachTop, reachBottom } = this._touchParams
        if (inner) {
          if (reachTop && this._refresh) {
            this._refresh.pullingEnd()
          }
          else if (reachBottom && this._loading) {
            this._loading.pullingEnd()
          }
        }
      }
      delete this._touchParams
    }
  }
}
