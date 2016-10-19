
export function createHandler (id, handler) {
  // @todo: The `callAddElement` API should be re-design immediately
  // because its public and global and without config opportunity.
  const defaultHandler = handler || callNative
  const hasAddElementHandler = typeof callAddElement === 'function'

  return function taskHandler (tasks) {
    if (!Array.isArray(tasks)) {
      tasks = [tasks]
    }
    for (let i = 0; i < tasks.length; i++) {
      const task = tasks[i]
      let returnValue
      if (hasAddElementHandler && task.module === 'dom' && task.method === 'addElement') {
        const [ref, json, index] = task.args
        returnValue = callAddElement(id, ref, json, index, '-1')
      }
      else {
        returnValue = defaultHandler(id, [task], '-1')
      }
      if (returnValue === -1) {
        return returnValue
      }
    }
  }
}
