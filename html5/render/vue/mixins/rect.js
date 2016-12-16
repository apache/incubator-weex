export default {
  methods: {
    reachTop () {
      const wrapper = this.$refs.wrapper
      return (!!wrapper) && (wrapper.scrollTop <= 0)
    },
    reachBottom () {
      const wrapper = this.$refs.wrapper
      const inner = this.$refs.inner
      const offset = Number(this.loadmoreoffset) || 0

      if (wrapper && inner) {
        const innerHeight = inner.getBoundingClientRect().height
        const wrapperHeight = wrapper.getBoundingClientRect().height
        return wrapper.scrollTop >= innerHeight - wrapperHeight - offset
      }
      return false
    }
  }
}
