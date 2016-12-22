export default {
  methods: {
    moveTo (offsetY = 0, done) {
      const inner = this.$refs.inner
      if (inner) {
        inner.style.willChange = `transform`
        inner.style.transition = `transform .2s ease-in-out`
        inner.style.transform = `translate3d(0, ${offsetY}, 0)`
        setTimeout(() => {
          inner.style.transition = ''
          inner.style.willChange = ''
          done && done()
        }, 200)
      }
    },

    done () {
      this.moveTo(0)
      this._refresh && this._refresh.reset()
      this._loading && this._loading.reset()
    },

    showRefresh () {
      // console.log('show refresh')
      this.moveTo('120px')
      if (this._refresh && this._refresh.child) {
        // console.log(this._refresh)
        this._refresh.child.show()
        // this._refresh.child.$emit('refresh', this.createCustomEvent('refresh'))
      }
    },

    showLoading () {
      // console.log('show loading')
      this.moveTo('-120px')
      if (this._loading && this._loading.child) {
        // this._loading.height = '120px'
        this._loading.child.show()
        // this.$emit('loading', this.createCustomEvent('loading'))
      }
    },

    handleTouchStart (event) {
      // console.log('list touch start')
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
      // console.log('touch move')
      if (this._touchParams) {
        const inner = this.$refs.inner
        const { startY, reachTop, reachBottom } = this._touchParams
        if (inner && (reachTop && this._refresh || reachBottom && this._loading)) {
          const touch = event.changedTouches[0]
          const offsetY = touch.pageY - startY
          // console.log('offsetY', offsetY, 'startY', startY, 'pageY', touch.pageY)
          this._touchParams.offsetY = offsetY
          if (offsetY) {
            inner.style.transform = `translate3d(0, ${offsetY}px, 0)`
          }
        }
      }
    },

    handleTouchEnd (event) {
      // event.preventDefault()
      event.stopPropagation()
      // console.log('touch end')
      if (this._touchParams) {
        const inner = this.$refs.inner
        const { offsetY, reachTop, reachBottom } = this._touchParams
        // console.log('offsetY:', offsetY)
        if (inner && (reachTop && this._refresh || reachBottom && this._loading)) {
          // this.moveTo(0)
          if (offsetY > 120) {
            this.showRefresh()
          }
          else if (offsetY < -120) {
            this.showLoading()
          }
          else {
            this.moveTo(0)
          }
        }
      }
      delete this._touchParams
    }
  }
}
