// input and textare has some common api and event
import { extend } from '../utils'

const findEnterKeyType = function (key) {
  const keys = ['default', 'go', 'next', 'search', 'send']
  if (keys.indexOf(key) > -1) {
    return key
  }
  return 'done'
}

export default {
  methods: {
    // support enter key envent
    createKeyboardEvent (events) {
      const customKeyType = this.returnKeyType
      const self = this
      if (this._events['return']) {
        const keyboardEvents = {
          'keyup': function (ev) {
            const code = ev.keyCode
            let key = ev.key
            if (code === 13) {
              if (key.toLowerCase() === 'tab') {
                key = 'next'
              }
              const rightKeyType = findEnterKeyType(customKeyType)
              ev.returnKeyType = rightKeyType
              ev.value = ev.target.value
              self.$emit('return', ev)
            }
          }
        }
        events = extend(events, keyboardEvents)
      }
      return events
    }
  }
}
