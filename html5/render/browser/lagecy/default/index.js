/**
 * @fileOverview Main entry, instance manager
 *
 * - createInstance(instanceId, code, options, data)
 * - refreshInstance(instanceId, data)
 * - destroyInstance(instanceId)
 * - registerComponents(components)
 * - registerModules(modules)
 * - getRoot(instanceId)
 * - instanceMap
 * - callJS(instanceId, tasks)
 *   - fireEvent(ref, type, data)
 *   - callback(funcId, data)
 */
export { createInstance } from '../static/create'
export { init, refreshInstance, destroyInstance } from '../../../../frameworks/legacy/static/life'
export { registerComponents, registerModules, registerMethods } from '../../../../frameworks/legacy/static/register'
export { receiveTasks } from '../../../../frameworks/legacy/static/bridge'
export { getRoot } from '../../../../frameworks/legacy/static/misc'
