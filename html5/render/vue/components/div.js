// import { validateStyles } from '../validator'

export default {
  name: 'weex-div',
  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('div', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    return createElement('html:div', {
      attrs: { 'weex-type': 'div' },
      on: this._createEventMap(),
      staticClass: 'weex-div weex-ct'
    }, this._trimTextNodeChildren(this.$slots.default))
  }
}
