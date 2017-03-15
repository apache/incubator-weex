export default {
  render (createElement) {
    this.weexType = 'loading-indicator'
    return createElement('mark', {
      attrs: { 'weex-type': 'loading-indicator' },
      staticClass: 'weex-loading-indicator weex-ct',
      staticStyle: this._normalizeInlineStyles(this.$vnode.data)
    })
  }
}
