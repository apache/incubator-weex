import { validateStyles } from '../../../validator'

export default {
  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('cell', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    return createElement('section', {
      attrs: { 'weex-type': 'cell' },
      staticClass: 'weex-cell'
    }, this.$slots.default)
  }
}
