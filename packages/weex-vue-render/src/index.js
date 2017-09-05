import weex from '../../../html5/render/vue'

import components from '../../../html5/render/vue/components'
import modules from '../../../html5/render/vue/modules'

const plugins = components.concat(modules)

plugins.forEach(function (plugin) {
  weex.install(plugin)
})

export default weex
