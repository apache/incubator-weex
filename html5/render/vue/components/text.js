/**
 * @fileOverview Impl of text component.
 *
 * Notes about the style 'height' and 'lines':
 * if the computed value of 'height' is bigger than 'lines', than the text will
 * be clipped according to the 'lines'. Otherwise, it'll be the 'height'.
 */

import { extractComponentStyle } from '../core'
import { extend } from '../utils'

/**
 * Get text special styles (lines and text-overflow).
 */
function getTextSpecStyle (ms = {}) {
  const lines = parseInt(ms.lines) || 0
  const overflow = ms['text-overflow'] || 'ellipsis'
  if (lines > 0) {
    return {
      overflow: 'hidden',
      'text-overflow': overflow,
      '-webkit-line-clamp': lines
    }
  }
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
    const style = extractComponentStyle(this)
    const textSpecStyle = getTextSpecStyle(style)
    return createElement('p', {
      attrs: { 'weex-type': 'text' },
      on: this._createEventMap(),
      staticClass: 'weex-text weex-el',
      staticStyle: extend(style, textSpecStyle)
    }, this.$slots.default || [this.value])
  }
}
