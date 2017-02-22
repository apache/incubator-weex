import { base } from '../mixins'
import { createEvent } from '../utils'
import { validateStyles } from '../validator'

export default {
  mixins: [base],
  props: {
    src: String
  },
  methods: {
    // TODO: check cross-origin
    goBack () {
      if (this.$el) {
        this.$el.contentWindow.history.back()
      }
    },
    goForward () {
      if (this.$el) {
        this.$el.contentWindow.history.forward()
      }
    },
    reload () {
      if (this.$el) {
        this.$el.contentWindow.history.reload()
      }
    }
  },

  mounted () {
    if (this.$el) {
      this.$emit('pagestart', createEvent(this.$el, 'pagestart', { url: this.src }))
      this.$el.addEventListener('load', event => {
        this.$emit('pagefinish', createEvent(this.$el, 'pagefinish', { url: this.src }))
      })
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('web', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    return createElement('iframe', {
      attrs: {
        'weex-type': 'web',
        src: this.src
      },
      on: this.createEventMap(['error']),
      staticClass: 'weex-web'
    })
  }
}
