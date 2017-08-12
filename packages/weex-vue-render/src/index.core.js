import weex from '../../../html5/render/vue'

import div from '../../../html5/render/vue/components/div'
import text from '../../../html5/render/vue/components/text'
import image from '../../../html5/render/vue/components/image'
import scrollable from '../../../html5/render/vue/components/scrollable'

const preInit = weex.init

weex.init = function () {
  preInit.apply(weex, arguments)
  weex.install(div)
  weex.install(text)
  weex.install(image)
  weex.install(scrollable)
}

if (global.Vue) {
  weex.init(global.Vue)
}

export default weex
