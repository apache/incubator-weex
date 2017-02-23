import { extend, extractKeys } from '../../utils'
import { base } from '../../mixins'

function getIndicatorItemStyle (spec, isActive) {
  const style = {}
  style['background-color'] = spec[isActive ? 'item-selected-color' : 'item-color']
  style['width'] = style['height'] = spec['item-size']
  return style
}

function _render (context, h) {
  const children = []
  const { mergedStyle } = context.$vnode.data
  context.$vnode.data.cached = {}
  extractKeys(context.$vnode.data.cached, mergedStyle, ['width', 'height'])
  const indicatorSpecStyle = extractKeys(
      {},
      mergedStyle,
      ['item-color', 'item-selected-color', 'item-size']
    )
  for (let i = 0; i < Number(context.count); ++i) {
    const classNames = ['weex-indicator-item']
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
  if (!context.$vnode.context._isMounted) {
    context.$nextTick(function () {
      _reLayout(this, _getVirtualRect(this), _getLtbr(this))
    })
  }
  return h('nav', {
    attrs: { 'weex-type': 'indicator' },
    staticClass: 'weex-indicator',
    staticStyle: mergedStyle
  }, children)
}

/**
 * get indicator's virtual rect (width, height), which is the .
 */
function _getVirtualRect (context) {
  const mergedStyle = context.$vnode.data.mergedStyle
  const ct = context.getParentRect()
  const rect = ['width', 'height'].reduce((pre, key) => {
    const msv = mergedStyle[key]
    pre[key] = msv ? parseFloat(msv) : ct[key]
    return pre
  }, {})
  return rect
}

/**
 * get indicator's ltbr values (without units).
 */
function _getLtbr (context) {
  const mergedStyle = context.$vnode.data.mergedStyle
  return ['left', 'top', 'bottom', 'right'].reduce((pre, key) => {
    const msv = mergedStyle[key]
    // undefined, null, or '0px' -> o
    pre[key] = msv && parseFloat(msv) || 0
    return pre
  }, {})
}

/**
 * get indicator's rect (width, height).
 */
function _getIndicatorRect (el) {
  let width, height
  if (el.children.length === 1) {
    width = height = window.getComputedStyle(el.children[0])
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
  const axisMap = [
    { dir: ltbr.left ? 'left' : ltbr.right ? 'right' : 'left', scale: 'width' },
    { dir: ltbr.top ? 'top' : ltbr.bottom ? 'bottom' : 'top', scale: 'height' }
  ]
  Object.keys(axisMap).forEach(key => {
    const { dir, scale } = axisMap[key]
    el.style[dir] = ltbr[dir] + virtualRect[scale] / 2 - rect[scale] / 2 + 'px'
  })
}

export default {
  name: 'indicator',
  mixins: [base],
  methods: {
    show: function () {
      this.$el.style.visibility = 'visible'
    }
  },
  props: {
    count: [Number, String],
    active: [Number, String]
  },
  updated () {
    _reLayout(this, _getVirtualRect(this), _getLtbr(this))
  },
  render (createElement) {
    // if (!this.$vnode.context._isMounted) {
    //   // return createElement('nav')
    //   return createElement('nav', {
    //     attrs: { 'weex-type': 'indicator' },
    //     staticClass: 'weex-indicator',
    //     staticStyle: { display: 'none' }
    //   }, [])
    // }
    this.prerender()
    return _render(this, createElement)
  }
}
