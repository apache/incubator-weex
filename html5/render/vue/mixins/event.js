import { extend } from '../utils'

export default {
  methods: {
    /**
     * Create Event.
     * @param {DOMString} type
     * @param {Object} props
     */
    createEvent (type, props) {
      const event = new Event(type, { bubbles: false })
      event.preventDefault()
      event.stopPropagation()

      extend(event, props)

      Object.defineProperty(event, 'target', {
        enumerable: true,
        value: this || null
      })

      return event
    },

    /**
     * Create Custom Event.
     * @param {DOMString} type
     * @param {Object} props
     */
    createCustomEvent (type, props) {
      // compatibility: http://caniuse.com/#search=customevent
      // const event = new CustomEvent(type)
      const event = document.createEvent('CustomEvent')
      event.initCustomEvent(type, false, true, {})
      event.preventDefault()
      event.stopPropagation()

      extend(event, props)

      Object.defineProperty(event, 'target', {
        enumerable: true,
        value: this || null
      })

      return event
    },

    /**
     * Check and emit longpress event.
     * @param {Object} event
     */
    handleLongPress (event) {
      // TODO: check the condition
      this.$emit('longpress', this.createCustomEvent('longpress'))
    },

    /**
     * Check and emit appear event.
     * @param {Object} event
     */
    handleAppear (event) {
      // TODO: check the condition
      this.$emit('appear', this.createCustomEvent('appear'))
    },

    /**
     * Check and emit disappear event.
     * @param {Object} event
     */
    handDisappear (event) {
      // TODO: check the condition
      this.$emit('disappear', this.createCustomEvent('disappear'))
    }
  }
}
