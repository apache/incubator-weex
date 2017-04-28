import weex from '../../../../render/vue/env/weex'
import * as utils from './utils'

const helper = {
  roots: {},
  _done: {},

  utils,
  /**
   * register a component.
   * @param  {string} name,
   * @param  {object} component.
   */
  register (name, component) {
    weex.registerComponent(name, component)
    // components[name] = component
  },

  /**
   * create a vm instance of Vue.
   * @param  {Object} options.
   * @return {Vue} vue instance.
   */
  createVm (options = {}, id) {
    // options.components = components
    let ct, root
    const Vue = weex.__vue__
    if (id) {
      ct = document.createElement('div')
      ct.id = `${id}-root`
      ct.style.cssText = 'width:100%;height:300px;overflow:scroll;'
      root = document.createElement('div')
      root.id = id
      ct.appendChild(root)
      document.body.appendChild(ct)
      this.roots[id] = root
    }
    return new Vue(options).$mount(root)
  },

  clearAll () {
    const roots = this.roots
    Object.keys(roots).forEach((id) => {
      const ct = roots[id].parentNode
      document.body.removeChild(ct)
    })
    this.roots = {}
  },

  clear (id) {
    if (!id) {
      return this.clearAll()
    }
    const roots = this.roots
    const root = roots[id]
    if (!root) { return }
    const ct = roots[id].parentNode
    document.body.removeChild(ct)
  },

  registerDone (id, cb) {
    this._done[id] = cb
  },

  unregisterDone (id) {
    if (!id) { return }
    delete this._done[id]
  },

  done (id, ...args) {
    const done = this._done[id]
    done && done(...args)
  },

  /**
   * [compile description]
   * @param  {[type]} template [description]
   * @return {[type]}          [description]
   */
  compile (template) {
    return helper.createVm({ template })
  }
}

export default helper
