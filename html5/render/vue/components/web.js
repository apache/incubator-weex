import { validateStyles } from '../validator'

export default {
  props: {
    src: String
  },
  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('web', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    return createElement('iframe', {
      attrs: {
        'weex-type': 'web',
        src: this.src
      },
      staticClass: 'weex-web'
    })
  }
}
