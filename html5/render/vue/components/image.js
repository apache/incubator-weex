export default {
  props: {
    src: String,
    resize: {
      validator (value) {
        /* istanbul ignore next */
        return ['cover', 'contain', 'stretch'].indexOf(value) !== -1
      }
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    if (!this.src && process.env.NODE_ENV === 'development') {
      console.warn(`[Vue Renderer] The <image> component must have the "src" attribute.`)
    }

    let cssText = `background-image:url("${this.src}");`

    // compatibility: http://caniuse.com/#search=background-size
    if (this.resize) {
      cssText += (this.resize === 'stretch')
      ? `background-size: 100% 100%;`
      : `background-size: ${this.resize};`
    }

    return createElement('figure', {
      attrs: { 'weex-type': 'image' },
      staticClass: 'weex-image',
      style: cssText
    })
  }
}
