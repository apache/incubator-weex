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

const _css = `
.weex-indicator {
  position: absolute;
  z-index: 10;
  -webkit-flex-direction: row;
  -ms-flex-direction: row;
  -moz-box-orient: horizontal;
  -moz-box-direction: normal;
  flex-direction: row;
  -webkit-box-orient: horizontal;
  margin: 0;
  padding: 0;
}

.weex-indicator-item {
  display: inline-block;
  position: relative;
  border-radius: 50%;
  width: 0.266667rem;
  height: 0.266667rem;
  background-color: #BBBBBB;
}
.weex-indicator-item + .weex-indicator-item {
  margin-left: 0.133333rem;
}

.weex-indicator-item-active {
  background-color: blue;
}
`

let extractComponentStyle, extend, extendKeys

function getIndicatorItemStyle (spec, isActive) {
  const style = {}
  style['background-color'] = spec[isActive ? 'itemSelectedColor' : 'itemColor']
  style['width'] = style['height'] = spec['itemSize']
  return style
}

function _render (context, h) {
  const children = []
  const mergedStyle = extractComponentStyle(context)
  const indicatorSpecStyle = extendKeys(
      {},
      mergedStyle,
      ['itemColor', 'itemSelectedColor', 'itemSize']
    )
  for (let i = 0; i < Number(context.count); ++i) {
    const classNames = ['weex-indicator-item weex-el']
    let isActive = false
    if (i === Number(context.active)) {
      classNames.push('weex-indicator-item-active')
      isActive = true
    }
    children.push(h('mark', {
      staticClass: classNames.join(' '),
      staticStyle: getIndicatorItemStyle(indicatorSpecStyle, isActive)
    }))
  }
  context.$nextTick(function () {
    _reLayout(this, _getVirtualRect(this, mergedStyle), _getLtbr(this, mergedStyle))
  })
  return h('nav', {
    attrs: { 'weex-type': 'indicator' },
    staticClass: 'weex-indicator weex-ct',
    staticStyle: mergedStyle
  }, children)
}

/**
 * get indicator's virtual rect (width, height), which is the .
 */
function _getVirtualRect (context, mergedStyle) {
  const ct = context._getParentRect()
  const rect = ['width', 'height'].reduce((pre, key) => {
    const msv = mergedStyle && mergedStyle[key]
    pre[key] = msv ? parseFloat(msv) : ct[key]
    return pre
  }, {})
  return rect
}

/**
 * get indicator's ltbr values (without units).
 */
function _getLtbr (context, mergedStyle) {
  return ['left', 'top', 'bottom', 'right'].reduce((pre, key) => {
    const msv = mergedStyle && mergedStyle[key]
    if (!msv && msv !== 0) { return pre }
    pre[key] = parseFloat(msv)
    return pre
  }, {})
}

/**
 * get indicator's rect (width, height).
 */
function _getIndicatorRect (el) {
  let width, height
  if (el.children.length === 1) {
    const itemComputedStyle = window.getComputedStyle(el.children[0])
    width = parseFloat(itemComputedStyle.width)
    height = parseFloat(itemComputedStyle.height)
  }
  else {
    const itemComputedStyle = window.getComputedStyle(el.children[1])
    const padding = parseFloat(itemComputedStyle.marginLeft)
    height = parseFloat(itemComputedStyle.height)
    width = el.children.length * (height + padding) - padding
  }
  return { width, height }
}

/**
 * calculate and reset indicator's width, height, and ltbr.
 * @param {object} virtualRect. width and height of indicator's virtual rect box.
 * @param {object} ltbr. the user specified left, top, bottom, right pixels (without units).
 */
function _reLayout (context, virtualRect, ltbr) {
  const el = context.$el
  const rect = _getIndicatorRect(el)
  const rectWithPx = Object.keys(rect).reduce((pre, key) => {
    pre[key] = rect[key] + 'px'
    return pre
  }, {})
  extend(el.style, rectWithPx)
  const axisMap = [{
    dir: ltbr.left || ltbr.left === 0
      ? 'left' : ltbr.right || ltbr.right === 0
      ? 'right' : 'left',
    scale: 'width'
  }, {
    dir: ltbr.top || ltbr.top === 0
      ? 'top' : ltbr.bottom || ltbr.bottom === 0
      ? 'bottom' : 'top',
    scale: 'height'
  }]
  Object.keys(axisMap).forEach(key => {
    const { dir, scale } = axisMap[key]
    el.style[dir] = (ltbr[dir] || 0) + virtualRect[scale] / 2 - rect[scale] / 2 + 'px'
  })
}

const indicator = {
  name: 'weex-indicator',
  methods: {
    show: function () {
      this.$el.style.visibility = 'visible'
    }
  },
  data () {
    return {
      count: 0,
      active: 0
    }
  },
  render (createElement) {
    const { count, active } = this.$vnode.data.attrs || {}
    this.count = count
    this.active = active
    if (!this.count) { return }
    return _render(this, createElement)
  },
  _css
}

export default {
  init (weex) {
    extractComponentStyle = weex.extractComponentStyle
    extend = weex.utils.extend
    extendKeys = weex.utils.extendKeys
    weex.registerComponent('indicator', indicator)
  }
}
