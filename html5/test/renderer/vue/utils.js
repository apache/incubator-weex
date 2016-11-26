const Vues = [
  require('./vender/vue-2.0.0.js'),
  require('./vender/vue-2.1.0.js')
]

/**
 * Describe tests for multiple versions of Vue.
 */
export function multiDescribe (title, fn) {
  return describe(title, () => {
    Vues.forEach(Vue => {
      describe(`Vue ${Vue.version}`, () => fn(Vue, {
        createVm (template) {
          return new Vue({ template }).$mount()
        }
      }))
    })
  })
}
