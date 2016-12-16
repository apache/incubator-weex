export default {
  name: 'loading-indicator',
  render (createElement) {
    return createElement('mark', {
      attrs: { 'weex-type': 'loading-indicator' },
      staticClass: 'weex-loading-indicator'
    })
  }
}
