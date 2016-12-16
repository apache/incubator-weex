export default {
  render () {
    // TODO: add tag nesting validation
    if (process.env.NODE_ENV === 'development') {
      const tag = this.$options._componentTag
      const parentTag = this.$parent.$options._componentTag
      console.warn(`[Vue Renderer] The <${tag}> can't be the child of <${parentTag}>.`)
    }
    return null
  }
}
