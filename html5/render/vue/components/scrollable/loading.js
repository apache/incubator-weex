import LoadingIndicator from './loading-indicator'

export default {
  // name: 'loading',
  components: { LoadingIndicator },
  data () {
    return {
      height: 0
    }
  },
  methods: {
    show () {
      this.$emit('loading')
      this.height = '120px'
      this.visibility = 'visible'
    },
    reset () {
      this.height = 0
      this.visibility = 'hidden'
      this.$emit('loadingfinish')
    }
  },
  render (createElement) {
    return createElement('aside', {
      ref: 'loading',
      attrs: { 'weex-type': 'loading' },
      style: { height: this.height, visibility: this.visibility },
      staticClass: 'weex-loading'
    }, this.$slots.default)
  }
}
