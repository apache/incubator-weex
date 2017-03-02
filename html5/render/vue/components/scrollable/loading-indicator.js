export default {
  name: 'loading-indicator',
  render (createElement) {
    const ms = this._getComponentStyle(this.$vnode.data)
    this.weexType = 'loading-indicator'
    return createElement('mark', {
      attrs: { 'weex-type': 'loading-indicator' },
      staticClass: 'weex-loading-indicator',
      staticStyle: ms
    })
  }
}
