import { expect } from 'chai'
import { multiDescribe } from '../utils'
import image from '../../../../render/vue/components/image.vue'

multiDescribe('<image> component', (Vue, helper) => {
  before(() => {
    helper.register('image', image)
  })

  after(() => {
    helper.reset()
  })

  it('create simple image component', () => {
    const vm = helper.compile(`<image>abc</image>`)

    // console.log(vm.$el)
    expect(vm.$el.tagName).to.be.equal('DIV')
    expect(vm.$el.className).to.be.equal('weex-container')
    expect(vm.$el.innerHTML).to.be.equal('<img class="weex-img weex-element">')

    const image = vm.$el.children[0]
    expect(image.tagName).to.be.equal('IMG')
    expect(image.className).to.be.equal('weex-img weex-element')
  })

  it('image component with src', () => {
    const vm = helper.compile(`<image src="http://dotwe.org">`)

    // console.log(vm.$el)
    expect(vm.$el.tagName).to.be.equal('DIV')
    expect(vm.$el.className).to.be.equal('weex-container')

    const image = vm.$el.children[0]
    expect(image.tagName).to.be.equal('IMG')
    expect(image.getAttribute('src')).to.be.equal('http://dotwe.org')
    expect(image.className).to.be.equal('weex-img weex-element')
  })
})
