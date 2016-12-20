import Base from './base'
import { validateStyles } from '../validator'

export default Base.extend({
  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('div', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    return createElement('html:div', {
      attrs: { 'weex-type': 'div' },
      on: this.createEventMap(),
      staticClass: 'weex-div'
    }, this.$slots.default)
  }
})
