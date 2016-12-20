import { validateStyles } from '../validator'

export default {
  props: {
    src: String,
    playStatus: {
      type: String,
      default: 'pause',
      validator (value) {
        return ['play', 'pause'].indexOf(value) !== -1
      }
    },
    // auto-play ?
    autoplay: {
      type: [String, Boolean],
      default: false
    },

    // playsinline: {
    //   type: [String, Boolean],
    //   default: false
    // },
    controls: {
      type: [String, Boolean],
      default: false
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('video', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    // TODO: support playStatus and autoplay
    return createElement('html:video', {
      attrs: {
        'weex-type': 'video',
        // autoplay: this.autoplay,
        controls: this.controls,
        src: this.src
      },
      staticClass: 'weex-video'
    })
  }
}
