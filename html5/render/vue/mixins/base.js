
const supportedEvents = [
  'click'
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
    createStyle () {}
  }
}
