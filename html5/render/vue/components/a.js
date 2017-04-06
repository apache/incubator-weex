import { extractComponentStyle } from '../core'
// import { validateStyles } from '../validator'

const defaultStyle = `
.weex-a {
  text-decoration: none;
}
`

export default {
  name: 'weex-a',
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
      staticClass: 'weex-a weex-ct',
      staticStyle: extractComponentStyle(this)
    }, this._trimTextNodeChildren(this.$slots.default))
  },
  _d_style: defaultStyle
}
