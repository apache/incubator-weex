import Base from './base'
import { validateStyles } from '../validator'

export default Base.extend({
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
    if (process.env.NODE_ENV === 'development') {
      validateStyles('image', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    /* istanbul ignore next */
    if (!this.src && process.env.NODE_ENV === 'development') {
      console.warn(`[Vue Renderer] The <image> component must have the "src" attribute.`)
    }

    let cssText = `background-image:url("${this.src}");`

    // compatibility: http://caniuse.com/#search=background-size
    cssText += (this.resize && this.resize !== 'stretch')
      ? `background-size: ${this.resize};`
      : `background-size: 100% 100%;`

    return createElement('figure', {
      attrs: { 'weex-type': 'image' },
      staticClass: 'weex-image',
      style: cssText
    })
  }
})
