import { extend } from '../utils'

const supportedEvents = [
  'click', 'longpress', 'appear', 'disappear'
  // 'touchstart', 'touchmove', 'touchend'
]

export default {
  methods: {
    createEventMap (extras = []) {
      const eventMap = {}
      supportedEvents.concat(extras).forEach(name => {
        eventMap[name] = event => this.$emit(name, event)
      })
      return eventMap
    },

    // get styles from vnode
    getClassStyle () {
      const context = this.$vnode.context
      if (context && context.$options) {
        const styles = context.$options.style
        const staticClass = this.$vnode.data ? this.$vnode.data.staticClass : null
        if (Array.isArray(staticClass)) {
          return staticClass.reduce((res, name) => {
            return extend(res, styles[name])
          }, {})
        }
        return {}
      }
      return {}
    }
  }
}
