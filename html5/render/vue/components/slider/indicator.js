export default {
  name: 'indicator',
  props: {
    count: [Number, String],
    active: [Number, String]
  },
  render (createElement) {
    const children = []
    for (let i = 0; i < Number(this.count); ++i) {
      const classNames = ['weex-indicator-item']
      if (i === Number(this.active)) {
        classNames.push('weex-indicator-item-active')
      }
      children.push(createElement('menuitem', {
        staticClass: classNames.join(' ')
      }))
    }
    return createElement('menu', {
      attrs: { 'weex-type': 'indicator' },
      staticClass: 'weex-indicator'
    }, children)
  }
}
