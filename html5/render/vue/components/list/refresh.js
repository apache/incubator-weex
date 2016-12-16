import indicator from './loading-indicator'

export default {
  name: 'refresh',
  data () {
    return {
      height: 0
    }
  },
  methods: {
    show () {
      this.$emit('refresh', 'abc')
      // console.log('will emit refresh')
      this.height = '120px'
      this.visibility = 'visible'
    },
    reset () {
      this.height = 0
      this.visibility = 'hidden'
      this.$emit('refreshfinish')
    }
  },
  render (createElement) {
    return createElement('aside', {
      attrs: { 'weex-type': 'refresh' },
      style: { height: this.height, visibility: this.visibility },
      staticClass: 'weex-refresh'
    }, [createElement(indicator)])
  }
}
