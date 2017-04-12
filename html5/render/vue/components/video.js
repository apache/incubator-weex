// import { validateStyles } from '../validator'
import { extractComponentStyle, createEventMap } from '../core'

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

    autoplay: {
      type: [String, Boolean],
      default: false
    },
    autoPlay: {
      type: [String, Boolean],
      default: false
    },

    playsinline: {
      type: [String, Boolean],
      default: false
    },
    controls: {
      type: [String, Boolean],
      default: false
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    // if (process.env.NODE_ENV === 'development') {
    //   validateStyles('video', this.$vnode.data && this.$vnode.data.staticStyle)
    // }

    // TODO: support playStatus
    return createElement('html:video', {
      attrs: {
        'weex-type': 'video',
        autoplay: (this.autoplay !== 'false' && this.autoplay !== false),
        autoPlay: (this.autoplay !== 'false' && this.autoplay !== false),
        controls: this.controls,
        src: this.src
      },
      on: createEventMap(this, ['start', 'pause', 'finish', 'fail']),
      staticClass: 'weex-video weex-el',
      staticStyle: extractComponentStyle(this)
    })
  }
}
