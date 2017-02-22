import { base } from '../mixins'
import { validateStyles } from '../validator'

function getImgStyle (context) {
  const stretch = '100% 100%'
  const resize = context.resize || stretch
  const bgSize = ['cover', 'contain', stretch].indexOf(resize) > -1 ? resize : stretch
  return { 'background-size': bgSize }
}

export default {
  mixins: [base],
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
    }
  },

  mounted: function () {
    this.fireLazyload()
  },

  render (createElement) {
    this.prerender()
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('image', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    // let cssText = `background-image:url("${this.src}");`

    // // compatibility: http://caniuse.com/#search=background-size
    // cssText += (this.resize && this.resize !== 'stretch')
    //   ? `background-size: ${this.resize};`
    //   : `background-size: 100% 100%;`

    return createElement('figure', {
      attrs: {
        'weex-type': 'image',
        'img-src': this.src,
        'img-placeholder': this.placeholder
      },
      on: this.createEventMap(['load', 'error']),
      staticClass: 'weex-image',
      staticStyle: getImgStyle(this)
      // style: cssText
    })
  }
}
