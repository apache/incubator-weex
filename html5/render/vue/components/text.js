// import { validateStyles } from '../validator'
import { extend } from '../utils'

/**
 * Get text special styles (lines and text-overflow).
 */
function getTextSpecStyle (context = {}) {
  const propLines = parseInt(context.lines) || 0
  const propOverflow = context.textOverflow || 'ellipsis'
  const data = context.$options._parentVnode.data
  const staticStyle = data && data.staticStyle || {}
  const lines = parseInt(staticStyle.lines) || propLines
  const overflow = staticStyle['text-overflow'] || propOverflow
  if (lines > 0) {
    return {
      overflow: 'hidden',
      'text-overflow': overflow,
      '-webkit-line-clamp': lines
    }
  }
  return staticStyle
}

export default {
  props: {
    lines: [Number, String],
    value: [String]
  },

  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('text', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    const ms = this._getComponentStyle(this.$vnode.data)

    return createElement('p', {
      attrs: { 'weex-type': 'text' },
      on: this._createEventMap(),
      staticClass: 'weex-text',
      staticStyle: extend(ms, getTextSpecStyle(this))
    }, this.$slots.default || [this.value])
  }
}
