import { validateStyles } from '../validator'

export default {
  props: {
    type: {
      type: String,
      default: 'text',
      validator (value) {
        return [
          'button', 'email', 'number', 'password', 'search',
          'tel', 'text', 'url', 'tel'
          // unsupported type:
          // checkbox, color, date, datetime, file, hidden, image,
          // month, radio, range, reset, submit, time, week,
        ].indexOf(value) !== -1
      }
    },
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
    maxlength: [String, Number]
  },

  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('input', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    // TODO: support disabled / autofocus
    return createElement('html:input', {
      attrs: {
        'weex-type': 'input',
        type: this.type,
        value: this.value,
        placeholder: this.placeholder,
        maxlength: this.maxlength
      },
      staticClass: 'weex-input'
    })
  }
}
