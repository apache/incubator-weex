// import { validateStyles } from '../../../validator'
import { extractComponentStyle, createEventMap } from '../../../core'

export default {
  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('cell', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    return createElement('section', {
      attrs: { 'weex-type': 'cell' },
      on: createEventMap(this),
      staticClass: 'weex-cell weex-ct',
      staticStyle: extractComponentStyle(this)
    }, this.$slots.default)
  }
}
