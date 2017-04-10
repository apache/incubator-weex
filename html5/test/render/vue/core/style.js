import { init } from '../helper/runtime'
import div from '../../../../render/vue/components/div'
import image from '../../../../render/vue/components/image'
import { init as initViewport } from '../../../../render/vue/env/viewport'

import scopedStyleBundle from '../data/build/dotvue/scoped-style'

// function toArray (arr) {
//   return Array.prototype.slice.call(arr)
// }

// function getVStyleSheetNodes () {
//   const regVStyleSheets = /((?:,?\s*\.[\w-]+\[data-v-\w+\](?::\w+)?)+)\s*({[^}]+)/
//   const nodes = toArray(document.styleSheets)
//     .filter(function (styleSheet) {
//       return regVStyleSheets.test(styleSheet.ownerNode.textContent)
//     }).map(function (styleSheet) {
//       return styleSheet.ownerNode
//     })
//   return nodes
// }

init('core style', (Vue, helper) => {
  const { scale } = initViewport()
  before(() => {
    helper.register('div', div)
    helper.register('image', image)
  })

  it('should get normalized merged styles.', function () {
    const vm = helper.createVm(scopedStyleBundle)
    const el = vm.$refs.foo.$el || vm.$refs.foo
    expect(el).to.be.ok
    const expectedMap = {
      width: 200 * scale + 'px',
      height: 200 * scale + 'px',
      backgroundColor: 'red'
    }
    const expectedDirection = {
      WebkitBoxDirection: 'normal',
      WebkitBoxOrient: 'horizontal',
      WebkitFlexDirection: 'row',
      flexDirection: 'row'
    }
    const expectedTransform = {
      WebkitTransform: `translate3d(${100 * scale + 'px'}, ${100 * scale + 'px'}, 0px)`,
      transform: `translate3d(${100 * scale + 'px'}, ${100 * scale + 'px'}, 0px)`
    }

    for (const k in expectedMap) {
      expect(el.style[k]).to.equal(expectedMap[k])
    }
    const directionRes = []
    for (const k in expectedDirection) {
      directionRes.push(el.style[k] === expectedDirection[k])
    }
    expect(directionRes).to.include(true)

    const transformRes = []
    for (const k in expectedTransform) {
      transformRes.push(el.style[k] === expectedTransform[k])
    }
    expect(transformRes).to.include(true)
  })
})
