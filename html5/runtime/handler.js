
const handlerMap = {
  addElement: 'callAddElement'
}

/**
 * Create task handler
 * @param {string} id
 * @param {function} handler
 * @return {function} taskHandler
 */
export function createHandler (id, handler) {
  const defaultHandler = handler || callNative

  return function taskHandler (tasks) {
    if (!Array.isArray(tasks)) {
      tasks = [tasks]
    }
    for (let i = 0; i < tasks.length; i++) {
      const returnValue = dispatchTask(id, tasks[i], defaultHandler)
      if (returnValue === -1) {
        return returnValue
      }
    }
  }
}

/**
 * Check if there is a corresponding available handler in the environment
 * @param {string} module
 * @param {string} method
 * @return {boolean}
 */
function hasAvailableHandler (module, method) {
  return module === 'dom'
    && handlerMap[method]
    && typeof global[handlerMap[method]] === 'function'
}

/**
 * Dispatch the task to the specified handler
 * @param {string} id
 * @param {object} task
 * @param {function} defaultHandler
 * @return {number} signal returned from native
 */
function dispatchTask (id, task, defaultHandler) {
  const { module, method, args } = task

  if (hasAvailableHandler(module, method)) {
    switch (method) {
      case 'addElement':
        return callAddElement(id, args[0], args[1], args[2], '-1')

      default:
        console.error(`[JS Runtime] "${method}" has no corresponding handler`)
    }
  }

  return defaultHandler(id, [task], '-1')
}
