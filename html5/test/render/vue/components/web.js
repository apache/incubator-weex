import { init } from '../helper'
import web from '../../../../render/vue/components/web'

init('<web> component', (Vue, helper) => {
  const { utils, compile } = helper
  before(() => {
    helper.register('web', web)
  })

  it('simple <web> component', () => {
    const vm = compile(`<web></web>`)

    // console.log(vm.$el)
    expect(vm.$el.tagName).to.be.equal('IFRAME')
    expect(utils.toArray(vm.$el.classList)).to.include.members(['weex-web', 'weex-el'])
  })

  it('use src attribute', () => {
    const vm = compile(`<web src="https://x.cn"></web>`)

    expect(vm.$el.tagName).to.be.equal('IFRAME')
    expect(vm.$el.getAttribute('src')).to.be.equal('https://x.cn')
    expect(utils.toArray(vm.$el.classList)).to.include.members(['weex-web', 'weex-el'])
  })
})
