// import { validateStyles } from '../validator'
import { extractComponentStyle, trimTextVNodes, createEventMap } from '../core'

const _css = `
body > .weex-div {
  min-height: 100%;
}
`

export default {
  name: 'weex-div',
  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('div', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    return createElement('html:div', {
      attrs: { 'weex-type': 'div' },
      on: createEventMap(this),
      staticClass: 'weex-div weex-ct',
      staticStyle: extractComponentStyle(this)
    }, trimTextVNodes(this.$slots.default))
  },
  _css
}
