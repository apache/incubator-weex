'use strict'

import Weex from './render'

import root from './base/root'
import div from './base/div'

// install the extended apis and components.
import extensions from './extend'

Weex.install(root)
Weex.install(div)

Weex.install(extensions)

export default Weex
