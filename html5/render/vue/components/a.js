// import { validateStyles } from '../validator'

export default {
  props: {
    href: String
  },
  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('a', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    return createElement('html:a', {
      attrs: {
        'weex-type': 'a',
        href: this.href
      },
      on: this._createEventMap(),
      staticClass: 'weex-a weex-ct'
    }, this.$slots.default)
  }
}
