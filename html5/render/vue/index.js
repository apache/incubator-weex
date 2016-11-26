import Vue from 'vue'
import components from './components'

export default Vue.use({
  install (Vue, options) {
    Vue.config.isReservedTag = () => false

    for (const name in components) {
      const Comp = components[name]
      Vue.component(name, Comp)
    }

    Vue.config.isReservedTag = tag => !!components[tag.toLowerCase()]
  }
})
