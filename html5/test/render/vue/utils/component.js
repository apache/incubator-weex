import { init } from '../helper/runtime'
import div from '../../../../render/vue/components/div'

import firstScreenAppearBundle from '../data/build/dotvue/first-screen-appear.js'

init('utils component', (Vue, helper) => {
  const spys = {
    'appear': sinon.spy(),
    'disappear': sinon.spy()
  }
  window._spy_first_screen_appear = function (evt) {
    const spy = spys[evt.type]
    spy && spy(evt)
  }

  before(() => {
    helper.register('div', div)
  })

  describe('watchAppear', function () {
    it('should work when mounted and updated.', function (done) {
      const id = 'test-first-screen-appear'
      helper.createVm(firstScreenAppearBundle, id)
      helper.registerDone(id, () => {
        const { appear: appearSpy, disappear: disappearSpy } = spys
        expect(appearSpy.callCount).to.equal(2)
        expect(disappearSpy.callCount).to.equal(1)
        expect(appearSpy.args[0][0].type).to.equal('appear')
        expect(appearSpy.args[1][0].type).to.equal('appear')
        expect(disappearSpy.args[0][0].type).to.equal('disappear')
        expect(appearSpy.args[0][0].direction).to.not.exist
        expect(appearSpy.args[1][0].direction).to.not.exist
        expect(disappearSpy.args[0][0].direction).to.not.exist
        done()
      })
    })
  })
})
