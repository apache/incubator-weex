import { init } from '../helper'
import a from '../../../../render/vue/components/a'
import div from '../../../../render/vue/components/div'

init('<a> component', (Vue, helper) => {
  const { compile, utils } = helper

  before(() => {
    helper.register('a', a)
    helper.register('div', div)
  })

  it('simple <a> component', () => {
    const vm = compile(`<a>abc</a>`)
    const el = vm.$el
    expect(el.tagName.toLowerCase()).to.equal('a')
    const classListArr = utils.toArray(el.classList)
    expect('weex-a').to.be.oneOf(classListArr)
    expect('weex-ct').to.be.oneOf(classListArr)
    expect(el.getAttribute('weex-type')).to.be.equal('a')
    expect(el.innerHTML).to.be.equal('')
  })

  it('<a> with href', () => {
    const href = '//m.taobao.com'
    const vm = compile(`<a href="${href}"></a>`)
    expect(vm.$el.getAttribute('href')).to.be.equal(href)
  })

  it('<a> with children <div>', () => {
    const href = '//m.taobao.com'
    const vm = compile(`<a href="${href}"><div></div></a>`)
    const el = vm.$el
    expect(el.children.length).to.be.equal(1)
    expect(el.children[0].tagName).to.match(/^(?:html:)?div$/i)
    expect(el.children[0].getAttribute('weex-type')).to.be.equal('div')
  })
})
