import { extend, mapFormEvents } from '../utils'
// import { validateStyles } from '../validator'

export default {
  props: {
    type: {
      type: String,
      default: 'text',
      validator (value) {
        return [
          'email', 'number', 'password', 'search', 'tel', 'text', 'url'
          // unsupported type:
          // button, checkbox, color, date, datetime, file, hidden, image,
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

  methods: {
    focus () {
      this.$el && this.$el.focus()
    },
    blur () {
      this.$el && this.$el.blur()
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('input', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    const ms = this._getComponentStyle(this.$vnode.data)
    return createElement('html:input', {
      attrs: {
        'weex-type': 'input',
        type: this.type,
        value: this.value,
        disabled: (this.disabled !== 'false' && this.disabled !== false),
        autofocus: (this.autofocus !== 'false' && this.autofocus !== false),
        placeholder: this.placeholder,
        maxlength: this.maxlength
      },
      domProps: {
        value: this.value
      },
      on: extend(this._createEventMap(), mapFormEvents(this)),
      staticClass: 'weex-input',
      staticStyle: ms
    })
  }
}
