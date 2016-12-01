
export default {
  functional: true,
  render (createElement, context) {
    return createElement(
      'html:div',
      { staticClass: 'weex-container' },
      context.children
    )
  }
}
