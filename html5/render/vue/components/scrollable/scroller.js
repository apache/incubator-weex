import { base, scrollable } from '../../mixins'
import { validateStyles } from '../../validator'
import { debounce, bind, extend } from '../../utils'
import * as shared from './shared'

export default {
  mixins: [base, scrollable],
  props: {
    scrollDirection: {
      type: [String],
      default: 'vertical',
      validator (value) {
        return ['horizontal', 'vertical'].indexOf(value) !== -1
      }
    },
    loadmoreoffset: {
      type: [String, Number],
      default: 0
    }
  },

  computed: {
    wrapperClass () {
      const classArray = ['weex-scroller', 'weex-scroller-wrapper']
      if (this.scrollDirection === 'horizontal') {
        classArray.push('weex-scroller-horizontal')
      }
      return classArray.join(' ')
    }
  },

  methods: {
    createChildren (h) {
      const slots = this.$slots.default || []
      this._cells = slots.filter(vnode => {
        if (!vnode.tag || !vnode.componentOptions) return false
        switch (vnode.componentOptions.tag) {
          case 'loading': this._loading = shared.createLoading(this, h); return false
          case 'refresh': this._refresh = shared.createRefresh(this, h); return false
        }
        return true
      })
      return [
        this._refresh,
        h('html:div', {
          ref: 'inner',
          staticClass: 'weex-scroller-inner'
        }, this._cells),
        this._loading
      ]
    }
  },

  render (createElement) {
    /* istanbul ignore next */
    if (process.env.NODE_ENV === 'development') {
      validateStyles('scroller', this.$vnode.data && this.$vnode.data.staticStyle)
    }

    this._cells = this.$slots.default || []
    this.$nextTick(() => {
      this.updateLayout()
    })

    return createElement('main', {
      ref: 'wrapper',
      attrs: { 'weex-type': 'scroller' },
      staticClass: this.wrapperClass,
      on: extend(this.createEventMap(), {
        scroll: debounce(bind(this.handleScroll, this), 100)
      })
    }, this.createChildren(createElement))
  }
}
