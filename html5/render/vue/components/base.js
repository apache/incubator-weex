const supportedEvents = [
  'click'
  // 'touchstart', 'touchmove', 'touchend'
]

/* eslint-disable no-undef */
export default Vue.extend({
  name: 'BaseVueComponent',
  methods: {
    createEventMap (extras = []) {
      const eventMap = {}
      supportedEvents.concat(extras).forEach(name => {
        eventMap[name] = event => this.$emit(name, event)
      })
      return eventMap
    },
    createStyle () {}
  },
  render () {
    console.warn('[Vue Renderer] Do not create base component directly.')
  }
})
