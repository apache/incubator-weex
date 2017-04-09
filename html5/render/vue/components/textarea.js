import { extractComponentStyle, createEventMap } from '../core'
import { inputCommon } from '../mixins'
import { extend, mapFormEvents } from '../utils'

// import { validateStyles } from '../validator'

const _css = `
.weex-textarea {
  font-size: 0.426667rem
}
.weex-textarea:focus {
  outline: none;
}
`

export default {
  mixins: [inputCommon],
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
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('textarea', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    const events = extend(createEventMap(this), mapFormEvents(this))
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
      domProps: {
        value: this.value
      },
      on: this.createKeyboardEvent(events),
      staticClass: 'weex-textarea weex-el',
      staticStyle: extractComponentStyle(this)
    })
  },
  _css
}
