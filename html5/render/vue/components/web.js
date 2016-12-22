import { base } from '../mixins'
import { validateStyles } from '../validator'

export default {
  mixins: [base],
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
      on: this.createEventMap(['pagestart', 'pagepause', 'error']),
      staticClass: 'weex-web'
    })
  }
}
