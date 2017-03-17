import { base, inputCommon } from '../mixins'
import { extend, mapFormEvents } from '../utils'
import { validateStyles } from '../validator'

export default {
  mixins: [base, inputCommon],
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
    },
    returnKeyType: String
  },

  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('textarea', this.$vnode.data && this.$vnode.data.staticStyle)
    }
    const events = extend(this.createEventMap(), mapFormEvents(this))
    return createElement('html:textarea', {
      attrs: {
        'weex-type': 'textarea',
        value: this.value,
        disabled: (this.disabled !== 'false' && this.disabled !== false),
        autofocus: (this.autofocus !== 'false' && this.autofocus !== false),
        placeholder: this.placeholder,
        rows: this.rows,
        'return-key-type': this.returnKeyType
      },
      on: this.createKeyboardEvent(events),
      staticClass: 'weex-textarea'
    }, this.value)
  }
}
