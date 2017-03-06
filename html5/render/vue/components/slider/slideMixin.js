import { createEvent/*, nextFrame*/ } from '../../utils'

const TRANSITION_TIME = 200

// trigger scroll event frequency.
// const scrollDam = 16

export default {
  methods: {
    // get standard index
    normalizeIndex (index) {
      const newIndex = (index + this.frameCount) % this.frameCount
      return Math.min(Math.max(newIndex, 0), this.frameCount - 1)
    },

    slideTo (index) {
      const newIndex = this.normalizeIndex(index)
      const inner = this.$refs.inner
      this.innerOffset += Math.sign(this.currentIndex - index) * this.wrapperWidth
      if (inner) {
        // const match = (inner.style.transform || inner.style.webkitTransform).match(/(\d+)px/)
        // const currentOffset = parseFloat(match[1])
        // TODO: will-change | set styles together
        inner.style.webkitTransition = `-webkit-transform .2s ease-in-out`
        inner.style.transition = `transform .2s ease-in-out`
        inner.style.webkitTransform = `translate3d(${this.innerOffset}px, 0, 0)`
        inner.style.transform = `translate3d(${this.innerOffset}px, 0, 0)`
        setTimeout(() => {
          inner.style.webkitTransition = ''
          inner.style.transition = ''
        }, TRANSITION_TIME)
      }
      // TODO: emit scroll event.
      // nextFrame()

      if (newIndex !== this.currentIndex) {
        this.currentIndex = newIndex
        // replace $el with { attr, style } is a legacy usage. Is it necessary to
        // do this ? Or just tell devers to use inline functions to access attrs ?
        this.$emit('change', createEvent(this.$el, 'change', {
          index: this.currentIndex
        }))
        setTimeout(() => { this.reorder() }, TRANSITION_TIME)
      }
    },

    reorder () {
      this.$nextTick(() => {
        const prevIndex = this.normalizeIndex(this.currentIndex - 1)
        const nextIndex = this.normalizeIndex(this.currentIndex + 1)
        // TODO: clone frame when prevIndex === nextIndex
        // if (prevIndex !== nextIndex) {
        // }
        const prevCell = this._cells[prevIndex]
        const nextCell = this._cells[nextIndex]
        if (prevCell && prevCell.elm) {
          const prevOffset = -this.wrapperWidth - this.innerOffset
          prevCell.elm.style.webkitTransform = `translate3d(${prevOffset}px, 0, 0)`
          prevCell.elm.style.transform = `translate3d(${prevOffset}px, 0, 0)`
        }
        if (nextCell && nextCell.elm) {
          const nextOffset = this.wrapperWidth - this.innerOffset
          nextCell.elm.style.webkitTransform = `translate3d(${nextOffset}px, 0, 0)`
          nextCell.elm.style.transform = `translate3d(${nextOffset}px, 0, 0)`
        }
      })
    },

    next () {
      this.slideTo(this.currentIndex + 1)
    },

    prev () {
      this.slideTo(this.currentIndex - 1)
    },

    handleTouchStart (event) {
      event.stopPropagation()
      const touch = event.changedTouches[0]
      this._touchParams = {
        originalTransform: this.$refs.inner.style.webkitTransform || this.$refs.inner.style.transform,
        startTouchEvent: touch,
        startX: touch.pageX,
        startY: touch.pageY,
        timeStamp: event.timeStamp
      }
    },

    handleTouchMove (event) {
      event.stopPropagation()
      const tp = this._touchParams
      if (!tp) { return }
      const { startX, startY } = this._touchParams
      const touch = event.changedTouches[0]
      const offsetX = touch.pageX - startX
      const offsetY = touch.pageY - startY
      tp.offsetX = offsetX
      tp.offsetY = offsetY
      let isV = tp.isVertical
      if (typeof isV === 'undefined') {
        isV = tp.isVertical = Math.abs(offsetX) < Math.abs(offsetY)
        if (!isV) {
          this.$emit('scrollstart', createEvent(this.$el, 'scrollstart', {}))
        }
      }
      // vertical scroll. just ignore it.
      if (isV) {
        return
      }
      // horizontal scroll. trigger scroll event.
      event.preventDefault()
      const inner = this.$refs.inner
      if (inner && offsetX) {
        // TODO: add throttle.
        this.$emit('scroll', createEvent(this.$el, 'scroll', {
          offsetXRatio: offsetX / this.wrapperWidth
        }))
        inner.style.transform = `translate3d(${this.innerOffset + offsetX}px, 0, 0)`
        inner.style.webkitTransform = `translate3d(${this.innerOffset + offsetX}px, 0, 0)`
      }
    },

    handleTouchEnd (event) {
      event.stopPropagation()
      const tp = this._touchParams
      if (!tp) { return }
      const isV = tp.isVertical
      if (typeof isV === 'undefined') {
        return
      }
      const inner = this.$refs.inner
      const { offsetX } = tp
      if (inner) {
        this.$emit('scrollend', createEvent(this.$el, 'scrollend'))
        // TODO: test the velocity if it's less than 0.2.
        const reset = Math.abs(offsetX / this.wrapperWidth) < 0.2
        const direction = offsetX > 0 ? 1 : -1
        const newIndex = reset ? this.currentIndex : (this.currentIndex - direction)
        this.slideTo(newIndex)
      }
      delete this._touchParams
    }
  }
}
