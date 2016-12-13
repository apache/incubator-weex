export default {
  methods: {
    slideTo (index) {
      // let newIndex = (index | 0) // % this.frameCount
      let newIndex = (index | 0) % this.frameCount // scroll to left
      newIndex = Math.max(newIndex, 0)
      newIndex = Math.min(newIndex, this.frameCount - 1)

      const offset = -newIndex * this.wrapperWidth
      const inner = this.$refs.inner

      if (inner) {
        // TODO: will-change | set styles together
        inner.style.transition = `transform .2s ease-in-out`
        inner.style.transform = `translate3d(${offset}px, 0, 0)`
        setTimeout(() => {
          inner.style.transition = ''
        }, 200)
      }
      if (newIndex !== this.currentIndex) {
        this.currentIndex = newIndex
        this.$emit('change', this.createEvent('change', {
          index: this.currentIndex
        }))
      }
    },

    next () {
      this.slideTo(this.currentIndex + 1)
    },

    prev () {
      this.slideTo(this.currentIndex - 1)
    },

    handleTouchStart (event) {
      event.preventDefault()
      event.stopPropagation()
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
      event.stopPropagation()
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
      event.stopPropagation()
      // console.log('touch end')
      const inner = this.$refs.inner
      if (this._touchParams) {
        const { offsetX } = this._touchParams
        // console.log('touch pageX:', touch.pageX, ', offsetX:', offsetX)
        if (inner) {
          const reset = Math.abs(offsetX / this.wrapperWidth) < 0.2
          const direction = offsetX > 0 ? 1 : -1
          const newIndex = reset ? this.currentIndex : (this.currentIndex - direction)

          // console.log('reset:', reset, ', newIndex:', newIndex)
          this.slideTo(newIndex)
        }
      }
      delete this._touchParams
    }
  }
}
