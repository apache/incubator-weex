import { extend } from '../../utils'
import { base } from '../../mixins'

function getIndicatorItemStyle (config, isActive) {
  const itemColor = config['item-color']
  const itemSelectedColor = config['item-selected-color']
  const itemSize = config['item-size']
  const style = {}
  style['background-color'] = isActive ? itemSelectedColor : itemColor
  style['width'] = style['height'] = itemSize
  return style
}

function _render (context, h) {
  const children = []
  // const _scopeId = context.getScopeId()
  const { mergedStyle } = context.$vnode.data
  for (let i = 0; i < Number(context.count); ++i) {
    const classNames = ['weex-indicator-item']
    let isActive = false
    if (i === Number(context.active)) {
      classNames.push('weex-indicator-item-active')
      isActive = true
    }
    children.push(h('mark', {
      staticClass: classNames.join(' '),
      staticStyle: getIndicatorItemStyle(mergedStyle, isActive)
    }))
  }
  return h('nav', {
    attrs: { 'weex-type': 'indicator' },
    staticClass: 'weex-indicator',
  }, children)
}

/**
 * calculate and reset indicator's top and left.
 */
function _reLayout (context) {
  const el = context.$el
  const mergedStyle = context.$vnode.data.mergedStyle
  const ctRect = extend({}, context.getParentRect())
  extend(ctRect, { left: 0, top: 0 })
  const { width: ctWidth, height: ctHeight, left, top } = ['width', 'height', 'left', 'top']
    .reduce((pre, name) => {
      pre[name] = parseFloat(mergedStyle[name] || ctRect[name])
      return pre
    }, {})
  let width, size

  if (el.children.length === 1) {
    width = size = window.getComputedStyle(el.children[0])
  }
  else {
    const itemComputedStyle = window.getComputedStyle(el.children[1])
    const padding = parseFloat(itemComputedStyle.marginLeft)
    size = parseFloat(itemComputedStyle.width)
    width = el.children.length * (size + padding) - padding
  }

  el.style.width = width + 'px'
  el.style.height = size + 'px'
  el.style.left = left + ctWidth / 2 - width / 2 + 'px'
  el.style.top = top + ctHeight / 2 - size / 2 + 'px'
}

export default {
  name: 'indicator',
  mixins: [base],
  props: {
    count: [Number, String],
    active: [Number, String]
  },
  mounted: function () {
    _reLayout(this)
  },
  render (createElement) {
    this.prerender()
    return _render(this, createElement)
  }
}
