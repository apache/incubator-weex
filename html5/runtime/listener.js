import { extend } from './vdom/operation'

export default function Listener (id, handler) {
  this.id = id
  this.batched = false
  this.updates = []
  if (typeof handler === 'function') {
    this.handler = handler
  }
  else {
    console.error('[JS Runtime] invalid parameter, handler must be a function')
  }
}

export function createAction (name, args = []) {
  return { module: 'dom', method: name, args: args }
}

extend(Listener.prototype, {
  createFinish (callback) {
    const handler = this.handler
    return handler([createAction('createFinish')], callback)
  },

  updateFinish (callback) {
    const handler = this.handler
    return handler([createAction('updateFinish')], callback)
  },

  refreshFinish (callback) {
    const handler = this.handler
    return handler([createAction('refreshFinish')], callback)
  },

  createBody (element) {
    const body = element.toJSON()
    const children = body.children
    delete body.children
    const actions = [createAction('createBody', [body])]
    if (children) {
      actions.push.apply(actions, children.map(child => {
        return createAction('addElement', [body.ref, child, -1])
      }))
    }
    return this.addActions(actions)
  },

  addElement (element, ref, index) {
    if (!(index >= 0)) {
      index = -1
    }
    return this.addActions(createAction('addElement', [ref, element.toJSON(), index]))
  },

  removeElement (ref) {
    if (Array.isArray(ref)) {
      const actions = ref.map((r) => createAction('removeElement', [r]))
      return this.addActions(actions)
    }
    return this.addActions(createAction('removeElement', [ref]))
  },

  moveElement (targetRef, parentRef, index) {
    return this.addActions(createAction('moveElement', [targetRef, parentRef, index]))
  },

  setAttr (ref, key, value) {
    const result = {}
    result[key] = value
    return this.addActions(createAction('updateAttrs', [ref, result]))
  },

  setStyle (ref, key, value) {
    const result = {}
    result[key] = value
    return this.addActions(createAction('updateStyle', [ref, result]))
  },

  setStyles (ref, style) {
    return this.addActions(createAction('updateStyle', [ref, style]))
  },

  addEvent (ref, type) {
    return this.addActions(createAction('addEvent', [ref, type]))
  },

  removeEvent (ref, type) {
    return this.addActions(createAction('removeEvent', [ref, type]))
  },

  handler (actions, cb) {
    return cb && cb()
  },

  addActions (actions) {
    const updates = this.updates
    const handler = this.handler

    if (!Array.isArray(actions)) {
      actions = [actions]
    }

    if (this.batched) {
      updates.push.apply(updates, actions)
    }
    else {
      return handler(actions)
    }
  }
})
