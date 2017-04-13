import { init } from '../helper'
import _switch from '../../../../render/vue/components/switch'

init('<switch> component', (Vue, helper) => {
  const { utils, compile } = helper

  before(() => {
    helper.register('switch', _switch)
  })

  it('simple <switch> component', () => {
    const vm = compile(`<switch></switch>`)

    // console.log(vm.$el)
    expect(vm.$el.tagName).to.be.equal('SPAN')
    expect(utils.toArray(vm.$el.classList)).to.include('weex-switch')
    expect(vm.$el.innerHTML).to.be.equal('<small class="weex-switch-inner"></small>')
  })

  it('disabled <switch>', () => {
    const vmA = compile(`<switch disabled="true"></switch>`)
    const vmB = compile(`<switch disabled></switch>`)
    const vmC = compile(`<switch disabled="disabled"></switch>`)

    expect(vmA.$el.className).to.match(/weex\-switch\-disabled/)
    expect(vmB.$el.className).to.match(/weex\-switch\-disabled/)
    expect(vmC.$el.className).to.match(/weex\-switch\-disabled/)
  })

  it('enabled <switch>', () => {
    const vmA = compile(`<switch></switch>`)
    const vmB = compile(`<switch disabled="false"></switch>`)

    expect(vmA.$el.className).to.be.equal('weex-switch')
    expect(vmB.$el.className).to.be.equal('weex-switch')
  })

  it('checked <switch>', () => {
    const vmA = compile(`<switch checked="true"></switch>`)
    const vmB = compile(`<switch checked></switch>`)
    const vmC = compile(`<switch checked="checked"></switch>`)

    expect(vmA.$el.className).to.match(/weex\-switch\-checked/)
    expect(vmB.$el.className).to.match(/weex\-switch\-checked/)
    expect(vmC.$el.className).to.match(/weex\-switch\-checked/)
  })

  it('toggle <switch>', () => {
    const vm = new Vue(_switch)

    expect(vm.isChecked).to.not.be.true
    vm.toggle()
    expect(vm.isChecked).to.be.true
    vm.toggle()
    expect(vm.isChecked).to.not.be.true
  })

  it('toggle & disabled <switch>', () => {
    const vm = new Vue(_switch)
    vm.isDisabled = true

    expect(vm.isChecked).to.not.be.true
    vm.toggle()
    expect(vm.isChecked).to.not.be.true
  })

  it('unchecked <switch>', () => {
    const vmA = compile(`<switch></switch>`)
    const vmB = compile(`<switch checked="false"></switch>`)

    expect(vmA.$el.className).to.be.equal('weex-switch')
    expect(vmB.$el.className).to.be.equal('weex-switch')
  })

  it('disabled & checked <switch>', () => {
    const vmA = compile(`<switch disabled checked></switch>`)
    const vmB = compile(`<switch disabled="disabled" checked="checked"></switch>`)

    expect(vmA.$el.className).to.match(/weex\-switch\-checked/)
    expect(vmA.$el.className).to.match(/weex\-switch\-disabled/)
    expect(vmB.$el.className).to.match(/weex\-switch\-checked/)
    expect(vmB.$el.className).to.match(/weex\-switch\-disabled/)
  })
})
