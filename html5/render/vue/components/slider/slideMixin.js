/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
import { createEvent/*, nextFrame*/, fireLazyload, addTransform } from '../../utils'

const TRANSITION_TIME = 200

const MAIN_SLIDE_OPACITY = 1

// trigger scroll event frequency.
// const scrollDam = 16

export default {
  updated () {
    const children = this.$children
    const len = children && children.length
    if (children && len > 0) {
      const indicator = children[len - 1]
      if (indicator.$options._componentTag === 'indicator'
        || indicator.$vnode.data.ref === 'indicator') {
        indicator._watcher.get()
      }
    }
    fireLazyload(this.$el, true)
  },

  methods: {
    // get standard index
    normalizeIndex (index) {
      const newIndex = (index + this.frameCount) % this.frameCount
      return Math.min(Math.max(newIndex, 0), this.frameCount - 1)
    },

    slideTo (index) {
      if (!this.infinite || this.infinite === 'false') {
        if (index === -1 || index > (this._cells.length - 1)) {
          this.slideTo(this.currentIndex)
          return
        }
      }

      const newIndex = this.normalizeIndex(index)
      const inner = this.$refs.inner
      const step = this._cells.length <= 1 ? 0 : this.currentIndex - index
      const sign = step === 0 ? 0 : step > 0 ? 1 : -1
      this.innerOffset += sign * this.wrapperWidth
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
        // replace $el with { attr, style } is a legacy usage. Is it necessary to
        // do this ? Or just tell devers to use inline functions to access attrs ?
        this.$emit('change', createEvent(this.$el, 'change', {
          index: this.currentIndex
        }))
        setTimeout(() => { this.reorder(newIndex) }, TRANSITION_TIME)
      }
    },
    order () {
      this.$nextTick(() => {
        for (let i = 1, l = this._cells.length; i < l; i++) {
          const nextElm = this._cells[i].elm
          const nextOffset = this.wrapperWidth * i
          nextElm.style.webkitTransform = `translate3d(${nextOffset}px, 0, 0)`
          nextElm.style.transform = `translate3d(${nextOffset}px, 0, 0)`
        }
        // this.reorder()
      })
    },
    reorder (newIndex) {
      if (!newIndex && newIndex !== 0) {
        newIndex = this.currentIndex
      }
      // dir: 'current' | 'prev' | 'next'
      const setPosition = (elm, dir) => {
        const scale = window.weex.config.env.scale
        let neighborScale = this.neighborScale
        let opacity = this.neighborAlpha
        let offsetX = -this.innerOffset
        let offsetY = 0
        if (dir === 'current') {
          elm.style.zIndex = 1
          neighborScale = this.currentItemScale
          opacity = MAIN_SLIDE_OPACITY
        }

        elm.style.visibility = 'visible'

        const origin = dir === 'prev' ? '100% 0' : '0 0'
        elm.style.webkitTransformOrigin = origin
        elm.style.transformOrigin = origin

        const sign = dir === 'current' ? 0 : dir === 'prev' ? -1 : 1
        offsetX = -this.innerOffset + sign * this.wrapperWidth
        if (this.isNeighbor) {
          offsetY = (1 - neighborScale) * this.wrapperHeight / 2
          elm.style.opacity = opacity
          if (dir === 'current') {
            offsetX += this.wrapperWidth * (1 - neighborScale) / 2
          }
          else {
            offsetX = offsetX - sign * this.neighborSpace * scale
          }
        }

        elm.style.width = this.wrapperWidth + 'px'
        const transObj = {
          translate: `translate3d(${offsetX}px, ${offsetY}px, 0px)`
        }
        if (this.isNeighbor) {
          transObj.scale = `scale(${neighborScale})`
        }
        addTransform(elm, transObj)
      }

      const removeClone = (clone, prevElm) => {
        // switch current page.
        setPosition(prevElm, 'current')
        // const curTransform = `translate3d(${-this.innerOffset}px, 0, 0)`
        // prevElm.style.transform = curTransform
        // prevElm.style.webkitTransform = curTransform
        // remove clone node.
        clone && clone.parentElement.removeChild(clone)
      }

      this.$nextTick(() => {
        if (this._cells.length <= 1) {
          return
        }
        if (!this.infinite || this.infinite === 'false') {
          this.order()
          return
        }
        const lastPrev = this._prevElm
        const prevIndex = this.normalizeIndex(newIndex - 1)
        const nextIndex = this.normalizeIndex(newIndex + 1)
        let prevElm = this._prevElm = this._cells[prevIndex].elm
        const nextElm = this._cells[nextIndex].elm
        const currentElm = this._cells[newIndex].elm

        // put current slide on the top.
        setPosition(currentElm, 'current')
        currentElm.style.webkitBoxAlign = 'center'
        currentElm.style.webkitAlignItems = 'center'
        currentElm.style.AlignItems = 'center'

        // clone prevCell if there are only tow slides.
        if (this._cells.length === 2) {
          this._clonePrev && removeClone(this._clonePrev, lastPrev)
          this._clonePrev = prevElm.cloneNode(true)
          this._clonePrev.classList.add('weex-slide-clone-prev')
          prevElm.parentElement.insertBefore(this._clonePrev, currentElm)
          if (!this._prevFired) {
            fireLazyload(this._clonePrev, true)
            this._prevFired = true
          }
          prevElm = this._clonePrev
        }

        setPosition(prevElm, 'prev')
        prevElm.style.webkitBoxAlign = 'end'
        prevElm.style.webkitAlignItems = 'flex-end'
        prevElm.style.AlignItems = 'flex-end'
        setPosition(nextElm, 'next')
        nextElm.style.webkitBoxAlign = 'start'
        nextElm.style.webkitAlignItems = 'flex-start'
        nextElm.style.AlignItems = 'flex-start'
        this.currentIndex = newIndex
      })
    },

    next () {
      this.slideTo(this.currentIndex + 1)
    },

    prev () {
      this.slideTo(this.currentIndex - 1)
    },

    handleTouchStart (event) {
      // event.stopPropagation()
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
      // event.stopPropagation()
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
      // event.stopPropagation()
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
