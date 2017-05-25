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
import './slider.css'
import {
  throttle,
  extend,
  createEvent,
  /* nextFrame, */
  fireLazyload,
  addTransform,
  copyTransform,
  bind
} from '../../utils'
import { extractComponentStyle, createEventMap } from '../../core'

// const TRANSITION_TIME = 200
const TRANSITION_TIME = 1000
const MAIN_SLIDE_OPACITY = 1

// trigger scroll event frequency.
// const scrollDam = 16

export default {
  created () {
    this._clones = []
    this.innerOffset = 0
    this._indicator = null
    this.$nextTick(() => {
      this._updateLayout()
    })
  },

  beforeUpdate () {
    this._updateLayout()
    // this._reorder()
  },

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

  mounted () {
    this._updateLayout()
    this._startAutoPlay()
    this._slideTo(this.currentIndex)
    // this._reorder()
    fireLazyload(this.$el, true)
  },

  methods: {
    _computeWrapperSize () {
      const wrapper = this.$refs.wrapper
      if (wrapper) {
        const rect = wrapper.getBoundingClientRect()
        this.wrapperWidth = rect.width
        this.wrapperHeight = rect.height
      }
    },

    _updateLayout () {
      this._computeWrapperSize()
      // const inner = this.$refs.inner
      // if (inner) {
      //   inner.style.width = this.wrapperWidth * this.frameCount + 'px'
      // }
    },

    _formatChildren (createElement) {
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
          staticClass: 'weex-slider-cell weex-ct'
        }, [vnode])
      })
      if (indicatorVnode) {
        indicatorVnode.data.attrs = indicatorVnode.data.attrs || {}
        indicatorVnode.data.attrs.count = cells.length
        indicatorVnode.data.attrs.active = this.currentIndex
        this._indicator = indicatorVnode
      }
      return cells
    },

    _renderSlides (createElement) {
      this._cells = this._formatChildren(createElement)
      this.frameCount = this._cells.length
      this._renderHook()
      return createElement(
        'nav',
        {
          ref: 'wrapper',
          attrs: { 'weex-type': this.isNeighbor ? 'slider-neighbor' : 'slider' },
          on: extend(createEventMap(this, ['scroll', 'scrollstart', 'scrollend']), {
            touchstart: this._handleTouchStart,
            touchmove: throttle(bind(this._handleTouchMove, this), 25),
            touchend: this._handleTouchEnd
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
    },

    // get standard index
    _normalizeIndex (index) {
      const newIndex = (index + this.frameCount) % this.frameCount
      return Math.min(Math.max(newIndex, 0), this.frameCount - 1)
    },

    _startAutoPlay () {
      if (!this.autoPlay || this.autoPlay === 'false') {
        return
      }
      if (this._autoPlayTimer) {
        clearTimeout(this._autoPlayTimer)
        this._autoPlayTimer = null
      }
      const interval = parseInt(this.interval)
      this._lastSlideTime = Date.now()

      const autoPlayFn = bind(function () {
        clearTimeout(this._autoPlayTimer)
        const now = Date.now()
        let nextTick = this._lastSlideTime + interval - now
        nextTick = nextTick > 100 ? nextTick : interval
        this._next()
        this._lastSlideTime = now
        this._autoPlayTimer = setTimeout(autoPlayFn, nextTick)
      }, this)

      this._autoPlayTimer = setTimeout(autoPlayFn, interval)
    },

    _stopAutoPlay () {
      if (this._autoPlayTimer) {
        clearTimeout(this._autoPlayTimer)
        this._autoPlayTimer = null
      }
    },

    _slideTo (index) {
      if (!this.infinite || this.infinite === 'false') {
        if (index === -1 || index > (this.frameCount - 1)) {
          this._slideTo(this.currentIndex)
          return
        }
      }

      const newIndex = this._normalizeIndex(index)
      const inner = this.$refs.inner
      let step = this._step = this.frameCount <= 1 ? 0 : this.currentIndex - index
      // newIndex: 0 -> 1, sign: -1; newIndex: 1 -> 0, sign: 1.
      const sign = step === 0 ? 0 : step > 0 ? 1 : -1

      if (Math.abs(step) > 1) {
        if (process.env.NODE_ENV === 'development') {
          console.warn('[vue-render] slider step should be no more than 1.')
        }
        step = sign
      }

      if (inner) {
        this._prepareNodes()

        this.innerOffset += sign * this.wrapperWidth
        // transform the whole slides group.
        // inner.style.webkitTransition = `-webkit-transform ${TRANSITION_TIME / 1000}s ease-in-out`
        // inner.style.transition = `transform ${TRANSITION_TIME / 1000}s ease-in-out`
        // inner.style.webkitTransform = `translate3d(${this.innerOffset}px, 0, 0)`
        // inner.style.transform = `translate3d(${this.innerOffset}px, 0, 0)`

        this._loopShowNodes(step)

        if (this.isNeighbor) {
          // set neighbors
          setTimeout(this._setNeighbors.bind(this), 0)
        }

        // current arrange index changed since the slides position begin to change.
        // this._currentIndexInArrangeArray = this._currentIndexInArrangeArray - step

        setTimeout(() => {
          // inner.style.webkitTransition = ''
          // inner.style.transition = ''
          for (let i = this._showStartIdx; i <= this._showEndIdx; i++) {
            const node = this._showNodes[i]
            if (!node) { continue }
            const elm = node.firstElementChild
            elm.style.webkitTransition = ''
            elm.style.transition = ''
          }

          // this._loopShowNodes(step)

          // clean cloned nodes and rearrange slide cells.
          this._rearrangeNodes(newIndex)
        }, TRANSITION_TIME)
      }

      // TODO: emit scroll event.
      // nextFrame()

      if (newIndex !== this.currentIndex) {
        // replace $el with { attr, style } is a legacy usage. Is it necessary to
        // do this ? Or just tell devers to use inline functions to access attrs ?
        this.$emit('change', createEvent(this.$el, 'change', {
          index: newIndex
        }))
        // setTimeout(() => { this._reorder(newIndex) }, TRANSITION_TIME)
      }
    },

    _loopShowNodes (step) {
      const sign = step > 0 ? 1 : -1
      let val = Math.abs(step)
      while (val--) {
        let i = sign < 0 ? this._showStartIdx : this._showEndIdx
        const end = sign < 0 ? this._showEndIdx : this._showStartIdx
        for (; i !== end - sign; i -= sign) {
          const nextIdx = i + sign
          this._positionNode(this._showNodes[i], nextIdx)
          // this._showNodes[nextIdx] = this._showNodes[i]
          // this._showNodes[nextIdx]._showIndex = nextIdx
          // this._positionNode(this._showNodes[nextIdx], nextIdx)
        }
      }
      const begin = sign < 0 ? this._showEndIdx : this._showStartIdx
      const end = begin - step
      for (let i = begin; i !== end; i -= sign) {
        delete this._showNodes[i]
      }
      this._showStartIdx += step
      this._showEndIdx += step
    },

    _prepareNodes () {
      // test if the next slide towards the direction exists.
      // e.g. currentIndex 0 -> 1: should prepare 4 slides: -1, 0, 1, 2
      // if not, translate a node to here, or just clone it.
      const step = this._step
      if (!this._inited) {
        this._initNodes()
        this._inited = true
        this._showNodes = {}
      //   // prepare nodes for the first time.
      //   // just generate the arrangeArray.
      //   for (let i = 0; i < total; i++) {
      //     const node = cells[i].elm
      //     node.index = i
      //     node.style.zIndex = 0
      //     node.style.opacity = 0
      //     node._showIndex = i
      //   }
      //   // this._positionNodes(start, end, cellBeginIndex)
      }

      const showCount = this._showCount = Math.abs(step) + 3

      this._showStartIdx = step <= 0 ? -1 : 1
      this._showEndIdx = step <= 0 ? showCount - 2 : 2 - showCount
      this._positionNodes(this._showStartIdx, this._showEndIdx, step)
    },

    _initNodes () {
      const total = this.frameCount
      const cells = this._cells
      for (let i = 0; i < total; i++) {
        const node = cells[i].elm
        node.index = i
        node._inShow = false
        node.style.zIndex = 0
        node.style.opacity = 0
      }
    },

    _positionNodes (begin, end, step) {
      const cells = this._cells
      const sign = step <= 0 ? -1 : 1
      let cellIndex = this.currentIndex + sign
      for (let i = begin; i !== end - sign; i = i - sign) {
        const node = cells[this._normalizeIndex(cellIndex)].elm
        cellIndex = cellIndex - sign
        this._positionNode(node, i)
      }
    },

    /**
     * index: position index in the showing cells' view.
     */
    _positionNode (node, index) {
      // if (this._showNodes[index]) {
      //   return
      // }
      if (node._inShow && this._showNodes[index] !== node) {
        node = this._getClone(node.index)
      }
      else if (node._inShow) {
        return
      }

      node._inShow = true
      // else if (!this._showNodes[index]) {
        // node = this._getClone(node.index)
      // }
      const translateX = index * this.wrapperWidth - this.innerOffset
      addTransform(node, {
        translate: `translate3d(${translateX}px, 0px, 0px)`
      })
      // node.style.left = (index + this._step) * this.wrapperWidth - this.innerOffset + 'px'
      // node.style.left = index * this.wrapperWidth + 'px'
      node.style.zIndex = 99 - Math.abs(index)
      node.style.opacity = 1
      node._showIndex = index
      this._showNodes[index] = node
    },

    _getClone (index) {
      let arr = this._clones[index]
      if (!arr) {
        this._clones[index] = arr = []
      }
      if (arr.length <= 0) {
        const origNode = this._cells[index].elm
        const clone = origNode.cloneNode(true)
        clone._isClone = true
        clone._inShow = origNode._inShow
        clone.index = origNode.index
        clone.style.left = 0
        clone.style.opacity = 0
        clone.style.zIndex = 0
        const ct = this.$refs.inner
        ct.appendChild(clone)
        arr.push(clone)
      }
      return arr.pop()
    },

    _removeClone (node) {
      const idx = node.index
      this._hideNode(node)
      const arr = this._clones[idx]
      arr.push(node)
    },

    _hideNode (node) {
      node._inShow = false
      node.style.opacity = 0
      node.style.left = 0
      node.style.zIndex = 0
    },

    /**
     * hide nodes from begin to end in showArray.
     * if it is clone node, just move the clone node to the buffer.
     */
    _clearNodes (begin, end) {
      for (let i = begin; i <= end; i++) {
        const node = this._showNodes[i]
        if (node._isClone) {
          this._removeClone(node)
        }
        else if (!node._inShow) {
          this._hideNode(node)
        }
        delete this._showNodes[i]
      }
    },

    /**
     * replace a clone node with the original node if it's not in use.
     */
    _replaceClone (clone, pos) {
      const origNode = this._cells[clone.index].elm
      if (origNode._inShow) {
        return
      }
      origNode._inShow = true
      origNode.style.left = clone.style.left
      origNode.style.opacity = clone.style.opacity
      origNode.style.zIndex = clone.style.zIndex
      copyTransform(clone, origNode)
      origNode.firstElementChild.style.left = clone.firstElementChild.style.left
      this._removeClone(clone)
      delete this._showNodes[clone._showIndex]
      this._showNodes[pos] = origNode
    },

    _rearrangeNodes (newIndex) {
      /**
       * clean nodes. replace current node with non-cloned node.
       * set current index to the new index.
       */
      const shows = this._showNodes

      for (let i = this._showStartIdx; i <= this._showEndIdx; i++) {
        shows[i]._inShow = false
      }
      for (let i = -1; i <= 1; i++) {
        const node = shows[i]
        if (!node._isClone) {
          node._inShow = true
        }
        else {
          this._replaceClone(node, i)
        }
      }

      this._clearNodes(this._showStartIdx, -2)
      this._clearNodes(2, this._showEndIdx)

      // const curNode = shows[0]
      // if (curNode._isClone) {
      //   // replace this cloned node with the original node.
      //   const idx = curNode.index
      //   for (let i = 0; i < nodesLen; i++) {
      //     if (nodes[i].index === idx) {
      //       break
      //     }
      //   }
      //   this._replaceCloneNode(curNode, i)
      // }

      // set current index to the new index.
      this.currentIndex = newIndex
    },

    /**
     * according to the attrs: neighborScale, neighborAlpha, neighborSpace.
     * 1. apply the main cell transform effects.
     * 2. set the previous cell and the next cell's positon, scale and alpha.
     * 3. set other cells' scale and alpha.
     */
    _setNeighbors () {
      // const curIdx = this._currentIndexInArrangeArray
      // const step = this._step
      // const newBegin = -1 - step

      for (let i = this._showStartIdx; i <= this._showEndIdx; i++) {
        const elm = this._showNodes[i].firstElementChild
        elm.style.webkitTransition = `all ${TRANSITION_TIME / 1000}s ease`
        elm.style.transition = `all ${TRANSITION_TIME / 1000}s ease`
        const transObj = {
          scale: `scale(${i === 0 ? this.currentItemScale : this.neighborScale})`
        }
        addTransform(elm, transObj)
        if (!this._neighborWidth) {
          this._neighborWidth = parseFloat(elm.style.width) || elm.getBoundingClientRect().width
        }
        // calculate position offsets according to neighbor scales.
        if (Math.abs(i) === 1) {
          const dist = (this.wrapperWidth - this._neighborWidth * this.neighborScale) / 2
            + this.neighborSpace * weex.config.env.scale
          elm.style.left = `${-i * dist}px`
        }
        // clear position offsets.
        else {
          elm.style.left = ''
        }
      }
    },

    _order () {
      this.$nextTick(() => {
        for (let i = 1, l = this._cells.length; i < l; i++) {
          const nextElm = this._cells[i].elm
          const nextOffset = this.wrapperWidth * i
          nextElm.style.webkitTransform = `translate3d(${nextOffset}px, 0, 0)`
          nextElm.style.transform = `translate3d(${nextOffset}px, 0, 0)`
        }
        // this._reorder()
      })
    },

    _reorder (newIndex) {
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
            // offsetX = offsetX - sign * this.neighborSpace * scale
            offsetX = offsetX - sign * (
              (this.wrapperWidth - this._origItemWidth * this.currentItemScale) / 2
              - this.neighborSpace * scale)
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
          this._order()
          return
        }
        const lastPrev = this._prevElm
        const prevIndex = this._normalizeIndex(newIndex - 1)
        const nextIndex = this._normalizeIndex(newIndex + 1)
        let prevElm = this._prevElm = this._cells[prevIndex].elm
        const nextElm = this._cells[nextIndex].elm
        const currentElm = this._cells[newIndex].elm

        if (!this._origItemWidth) {
          this._origItemWidth = currentElm.firstElementChild.getBoundingClientRect().width
        }

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

    _next () {
      this._slideTo(this.currentIndex + 1)
    },

    _prev () {
      this._slideTo(this.currentIndex - 1)
    },

    _handleTouchStart (event) {
      // event.stopPropagation()
      const touch = event.changedTouches[0]
      this._stopAutoPlay()
      this._touchParams = {
        originalTransform: this.$refs.inner.style.webkitTransform || this.$refs.inner.style.transform,
        startTouchEvent: touch,
        startX: touch.pageX,
        startY: touch.pageY,
        timeStamp: event.timeStamp
      }
    },

    _handleTouchMove (event) {
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

    _handleTouchEnd (event) {
      // event.stopPropagation()
      this._startAutoPlay()
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
        this._slideTo(newIndex)
      }
      delete this._touchParams
    }
  }
}
