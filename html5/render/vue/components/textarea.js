import Base from './base'
import { validateStyles } from '../validator'

export default Base.extend({
  props: {
    value: String,
    placeholder: String,
    disabled: {
      type: [String, Boolean],
      default: false
    },
    autofocus: {
      type: [String, Boolean],
      default: false
    },
    rows: {
      type: [String, Number],
      default: 2
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('textarea', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    // TODO: support disabled / autofocus / rows
    return createElement('html:textarea', {
      attrs: {
        'weex-type': 'textarea',
        value: this.value,
        placeholder: this.placeholder,
        rows: this.rows
      },
      staticClass: 'weex-textarea'
    })
  }
})
