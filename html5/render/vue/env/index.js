import '../styles/reset.css'
import '../styles/components.css'

import '../../../shared/arrayFrom'
import '../../../shared/objectAssign'
import '../../../shared/objectSetPrototypeOf'

import 'core-js/modules/es6.object.to-string'
import 'core-js/modules/es6.string.iterator'
import 'core-js/modules/web.dom.iterable'
import 'core-js/modules/es6.promise'

import './WXEnvironment'
export * from './viewport'

import api from '../../browser/extend/api'
import * as weex from './weex'

api.init(weex)

Object.freeze(weex)
window.weex = weex
