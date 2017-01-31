import App from '../app/index'
import { instanceMap } from './map'
import { init as initApp } from '../app/ctrl/index'
import { resetTarget } from '../core/dep'

/**
 * Create a Weex instance.
 *
 * @param  {string} id
 * @param  {string} code
 * @param  {object} options
 *         option `HAS_LOG` enable print log
 * @param  {object} data
 * @param  {object} info { created, ... services, callbacks }
 */
export function createInstance (id, code, options, data, info) {
  const { services, callbacks, weex } = info || {}
  resetTarget()
  let instance = instanceMap[id]
  /* istanbul ignore else */
  options = options || {}
  let result
  /* istanbul ignore else */
  if (!instance) {
    instance = new App(id, options, callbacks, weex)
    instanceMap[id] = instance
    result = initApp(instance, code, data, services, weex)
  }
  else {
    result = new Error(`invalid instance id "${id}"`)
  }
  return result
}
