
export default {
  methods: {
    updateLayout () {
      this.computeWrapperSize()
      if (this._cells && this._cells.length) {
        this._cells.forEach(vnode => {
          vnode._visible = this.isCellVisible(vnode.elm)
        })
      }
    },

    isCellVisible (elem) {
      if (!this.wrapperHeight) {
        this.computeWrapperSize()
      }
      const wrapper = this.$refs.wrapper
      return wrapper.scrollTop <= elem.offsetTop
        && elem.offsetTop < wrapper.scrollTop + this.wrapperHeight
    },

    computeWrapperSize () {
      const wrapper = this.$refs.wrapper
      if (wrapper) {
        const rect = wrapper.getBoundingClientRect()
        this.wrapperWidth = rect.width
        this.wrapperHeight = rect.height
      }
    },

    handleScroll (event) {
      this._cells.forEach((vnode, index) => {
        const visible = this.isCellVisible(vnode.elm)
        if (visible !== vnode._visible) {
          const type = visible ? 'appear' : 'disappear'
          vnode._visible = visible

          // TODO: dispatch CustomEvent
          vnode.elm.dispatchEvent(new Event(type), {})
        }
      })
      if (this.reachBottom()) {
        this.$emit('loadmore', event)
      }
    },

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
