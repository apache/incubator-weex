import { init } from '../helper'
import div from '../../../../render/vue/components/div'

init('<div> component', (Vue, helper) => {
  const { compile, utils } = helper

  before(() => {
    helper.register('div', div)
  })

  it('simple <div> component', () => {
    const vm = compile(`<div>abc</div>`)
    const el = vm.$el
    expect(el.tagName.toLowerCase()).to.equal('div')
    const classListArr = utils.toArray(el.classList)
    expect('weex-div').to.be.oneOf(classListArr)
    expect('weex-ct').to.be.oneOf(classListArr)
    expect(el.getAttribute('weex-type')).to.be.equal('div')
    expect(el.innerHTML).to.be.equal('')
  })

  it('<div> with children <div>', () => {
    const vm = compile(`<div><div></div></div>`)
    const el = vm.$el
    expect(el.children.length).to.be.equal(1)
    expect(el.children[0].tagName).to.match(/^(?:html:)?div$/i)
    expect(el.children[0].getAttribute('weex-type')).to.be.equal('div')
  })
})
