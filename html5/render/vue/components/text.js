// import { validateStyles } from '../validator'
import { extend } from '../utils'

/**
 * Get text special styles (lines and text-overflow).
 */
function getTextSpecStyle (context = {}) {
  // const propLines = parseInt(context.lines) || 0
  // const propOverflow = context.textOverflow || 'ellipsis'
  const data = context.$vnode.data
  const staticStyle = data.staticStyle || {}
  const styles = extend(staticStyle, data.style || {})
  const lines = parseInt(styles.lines) || 0
  const overflow = styles['text-overflow'] || 'ellipsis'
  if (lines > 0) {
    extend(styles, {
      overflow: 'hidden',
      'text-overflow': overflow,
      '-webkit-line-clamp': lines
    })
  }
  return styles
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
    return createElement('p', {
      attrs: { 'weex-type': 'text' },
      on: this._createEventMap(),
      staticClass: 'weex-text',
      staticStyle: getTextSpecStyle(this)
    }, this.$slots.default || [this.value])
  }
}
