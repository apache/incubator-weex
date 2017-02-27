import { extend } from '../utils'
// import { validateStyles } from '../validator'

/**
 * get resize (stetch|cover|contain) related styles.
 */
function getResizeStyle (context) {
  const stretch = '100% 100%'
  const resize = context.resize || stretch
  const bgSize = ['cover', 'contain', stretch].indexOf(resize) > -1 ? resize : stretch
  return { 'background-size': bgSize }
}

export default {
  props: {
    src: {
      type: String,
      required: true
    },
    placeholder: {
      type: String
    },
    resize: {
      validator (value) {
        /* istanbul ignore next */
        return ['cover', 'contain', 'stretch'].indexOf(value) !== -1
      }
    },
    quality: String,
    sharpen: String,
    original: [String, Boolean]
  },

  mounted () {
    this._fireLazyload()
  },

  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('image', this.$vnode.data && this.$vnode.data.staticStyle)
    // }

    // let cssText = `background-image:url("${this.src}");`

    // // compatibility: http://caniuse.com/#search=background-size
    // cssText += (this.resize && this.resize !== 'stretch')
    //   ? `background-size: ${this.resize};`
    //   : `background-size: 100% 100%;`
    return createElement('figure', {
      attrs: {
        'weex-type': 'image',
        'img-src': this._preProcessSrc(this.src),
        'img-placeholder': this._preProcessSrc(this.placeholder)
      },
      on: this._createEventMap(['load', 'error']),
      staticClass: 'weex-image'
    })
  },

  methods: {
    _preProcessSrc (url) {
      const { width, height } = this.$vnode.data.staticStyle
      return this.processImgSrc && this.processImgSrc(url, {
        width: parseFloat(width),
        height: parseFloat(height),
        quality: this.quality,
        sharpen: this.sharpen,
        original: this.original
      }) || url
    },
    beforeRender () {
      extend(this.$options._parentVnode.data.staticStyle, getResizeStyle(this))
    }
  }
}
