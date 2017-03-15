// import { validateStyles } from '../../../validator'

export default {
  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('cell', this.$vnode.data && this.$vnode.data.staticStyle)
    // }
    // const ms = this._getComponentStyle(this.$vnode.data)
    return createElement('section', {
      attrs: { 'weex-type': 'cell' },
      on: this._createEventMap(),
      staticClass: 'weex-cell weex-ct',
      // staticStyle: ms
      staticStyle: this._normalizeInlineStyles(this.$vnode.data)
    }, this.$slots.default)
  }
}
