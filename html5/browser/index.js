'use strict'

import Weex from './render'

/**
 * install components and APIs
 */
import root from './base/root'
import div from './base/div'
import droot from './base/droot'
import components from './extend/components'
import api from './extend/api'

Weex.install(root)
Weex.install(div)
Weex.install(droot)
Weex.install(components)
Weex.install(api)

export default Weex
