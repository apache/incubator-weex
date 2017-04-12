// import { validateStyles } from '../validator'
import { extractComponentStyle } from '../core'

const _css = `
.weex-switch {
  border: 0.013333rem solid #dfdfdf;
  cursor: pointer;
  display: inline-block;
  position: relative;
  vertical-align: middle;
  -webkit-user-select: none;
     -moz-user-select: none;
      -ms-user-select: none;
          user-select: none;
  box-sizing: content-box;
  background-clip: content-box;
  color: #64bd63;
  width: 1.333333rem;
  height: 0.8rem;
  background-color: white;
  border-color: #dfdfdf;
  box-shadow: #dfdfdf 0 0 0 0 inset;
  border-radius: 0.8rem;
  -webkit-transition: border 0.4s, box-shadow 0.4s, background-color 1.2s;
          transition: border 0.4s, box-shadow 0.4s, background-color 1.2s;
}

.weex-switch-checked {
  background-color: #64bd63;
  border-color: #64bd63;
  box-shadow: #64bd63 0 0 0 0.533333rem inset;
}

.weex-switch-checked.weex-switch-disabled {
  background-color: #A0CCA0;
  box-shadow: #A0CCA0 0 0 0 0.533333rem inset;
}

.weex-switch-disabled {
  background-color: #EEEEEE;
}

.weex-switch-inner {
  width: 0.8rem;
  height: 0.8rem;
  background: #fff;
  border-radius: 100%;
  box-shadow: 0 0.013333rem 0.04rem rgba(0, 0, 0, 0.4);
  position: absolute;
  top: 0;
  left: 0;
  -webkit-transition: background-color 0.4s, left 0.2s;
          transition: background-color 0.4s, left 0.2s;
}

.weex-switch-checked > .weex-switch-inner {
  left: 0.533333rem;
}
`

export default {
  props: {
    checked: {
      type: [Boolean, String],
      default: false
    },
    disabled: {
      type: [Boolean, String],
      default: false
    }
  },
  data () {
    return {
      isChecked: (this.checked !== 'false' && this.checked !== false),
      isDisabled: (this.disabled !== 'false' && this.disabled !== false)
    }
  },
  computed: {
    wrapperClass () {
      const classArray = ['weex-switch']
      this.isChecked && classArray.push('weex-switch-checked')
      this.isDisabled && classArray.push('weex-switch-disabled')
      return classArray.join(' ')
    }
  },
  methods: {
    toggle () {
      // TODO: handle the events
      if (!this.isDisabled) {
        this.isChecked = !this.isChecked
        this.$emit('change', { value: this.isChecked })
      }
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('switch', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    return createElement('span', {
      attrs: { 'weex-type': 'switch' },
      on: {
        click: event => {
          this.$emit('click', event)
          this.toggle()
        }
      },
      staticClass: this.wrapperClass,
      staticStyle: extractComponentStyle(this)
    }, [createElement('small', { staticClass: 'weex-switch-inner' })])
  },
  _css
}
