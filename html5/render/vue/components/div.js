import { validateStyles } from '../validator'

export default {
  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('div', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    return createElement('html:div', {
      attrs: { 'weex-type': 'div' },
      staticClass: 'weex-div'
    }, this.$slots.default)
  }
}
