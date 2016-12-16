import indicator from './loading-indicator'

export default {
  name: 'loading',
  data () {
    return {
      height: 0
    }
  },
  methods: {
    show () {
      this.$emit('loading')
      console.log('will emit loading')
      this.height = '120px'
      this.visibility = 'visible'
      console.log(this, this.height)
    },
    reset () {
      this.height = 0
      this.visibility = 'hidden'
      this.$emit('loadingfinish')
    }
  },
  render (createElement) {
    return createElement('aside', {
      ref: 'indicator',
      attrs: { 'weex-type': 'loading' },
      style: { height: this.height, visibility: this.visibility },
      staticClass: 'weex-loading'
    }, [createElement(indicator)])
  }
}
