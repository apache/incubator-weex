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
import { getThrottleLazyload, throttle } from '../utils'

const DEFAULT_OFFSET_ACCURACY = 10
const DEFAULT_LOADMORE_OFFSET = 0

function getThrottledScroll (context) {
  const scale = weex.config.env.scale
  if (!context._throttleScroll) {
    const wrapper = context.$refs.wrapper
    const inner = context.$refs.inner
    let preOffset = (context.scrollDirection === 'horizontal'
        ? wrapper.scrollLeft
        : wrapper.scrollTop)
      || 0
    context._throttleScroll = throttle(function (evt) {
      const offset = context.scrollDirection === 'horizontal'
        ? wrapper.scrollLeft
        : wrapper.scrollTop
      const indent = parseInt(context.offsetAccuracy) * scale
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
      if (Math.abs(offset - preOffset) >= indent) {
        triggerScroll()
        preOffset = offset
      }
    }, 16, true)
  }
  return context._throttleScroll
}

export default {
  props: {
    loadmoreoffset: {
      type: [String, Number],
      default: DEFAULT_LOADMORE_OFFSET,
      validator (value) {
        const val = parseInt(value)
        return !isNaN(val) && val >= DEFAULT_LOADMORE_OFFSET
      }
    },

    offsetAccuracy: {
      type: [Number, String],
      default: DEFAULT_OFFSET_ACCURACY,
      validator (value) {
        const val = parseInt(value)
        return !isNaN(val) && val >= DEFAULT_OFFSET_ACCURACY
      }
    }
  },

  created () {
    // should call resetLoadmore() to enable loadmore event.
    this._loadmoreReset = true
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

    resetLoadmore () {
      this._loadmoreReset = true
    },

    handleScroll (event) {
      getThrottleLazyload(25, this.$el, 'scroll')()
      getThrottledScroll(this)(event)

      // fire loadmore event.
      const inner = this.$refs.inner
      if (inner) {
        const innerRect = inner.getBoundingClientRect()
        const innerLength = this.scrollDirection === 'horizontal'
          ? innerRect.width
          : innerRect.height
        // hscroller not support loadmore event yet.
        if (this.scrollDirection === 'horizontal') {
          return
        }
        if (!this._innerLength) {
          this._innerLength = innerLength
        }
        if (this._innerLength !== innerLength) {
          console.log(this._innerLength, innerLength)
          this._innerLength = innerLength
          this._loadmoreReset = true
        }
        if (this._loadmoreReset && this.reachBottom()) {
          this._loadmoreReset = false
          this.$emit('loadmore', event)
        }
      }
    },

    reachTop () {
      const wrapper = this.$refs.wrapper
      return (!!wrapper) && (wrapper.scrollTop <= 0)
    },

    reachBottom () {
      const wrapper = this.$refs.wrapper
      const inner = this.$refs.inner
      const offset = parseInt(this.loadmoreoffset) * weex.config.env.scale

      if (wrapper && inner) {
        const innerRect = inner.getBoundingClientRect()
        const wrapperRect = wrapper.getBoundingClientRect()
        const key = this.scrollDirection === 'horizontal'
          ? 'width'
          : 'height'
        const innerLength = innerRect[key]
        const wrapperLength = wrapperRect[key]
        const scrollOffset = this.scrollDirection === 'horizontal'
          ? wrapper.scrollLeft
          : wrapper.scrollTop
        return scrollOffset >= innerLength - wrapperLength - offset
      }
      return false
    },

    handleTouchStart (event) {
      // event.preventDefault()
      // event.stopPropagation()
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
      // event.stopPropagation()
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
      // event.stopPropagation()
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
