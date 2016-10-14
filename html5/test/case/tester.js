import chai from 'chai'
import sinon from 'sinon'
import sinonChai from 'sinon-chai'

const expect = chai.expect
chai.use(sinonChai)

import { createRuntime, createApp, getCode } from './prepare'

describe('test input and output', function () {
  let runtime

  before(() => {
    runtime = createRuntime()
    sinon.stub(console, 'info')
    sinon.stub(console, 'warn')
    sinon.stub(console, 'error')
  })

  after(() => {
    console.info.restore()
    console.warn.restore()
    console.error.restore()
  })

  describe('one-time rendering cases', () => {
    let app

    beforeEach(() => {
      app = createApp(runtime)
      callNativeHandler = (...args) => app._target.callNative(...args)
    })

    afterEach(() => {
      callNativeHandler = function () {}
    })

    function checkOutput (app, name) {
      const source = getCode('assets/' + name + '.input')
      const output = getCode('assets/' + name + '.output')
      const result = eval('(' + output + ')')

      app.$create(source)
      expect(app.getRealRoot()).eql(result)
      app.$destroy()
    }

    it('single case', () => checkOutput(app, 'foo'))
    it('foo2 case', () => checkOutput(app, 'foo2'))
    it('foo3 case', () => checkOutput(app, 'foo3'))
    it('foo4 case', () => checkOutput(app, 'foo4'))
    it('foo5 case', () => checkOutput(app, 'foo5'))
    it('foo6 case', () => checkOutput(app, 'foo6'))
    it('foo7 case', () => checkOutput(app, 'foo7'))
    it('foo8 case', () => checkOutput(app, 'foo8'))
    it('foo9 case', () => checkOutput(app, 'foo9'))

    it('static1 case', () => checkOutput(app, 'static1'))
    it('static2 case', () => checkOutput(app, 'static2'))
    it('static3 case', () => checkOutput(app, 'static3'))
    it('static4 case', () => checkOutput(app, 'static4'))

    it('computed in repeat case', () => checkOutput(app, 'computed-in-repeat'))
    it('backward(register/render) case', () => checkOutput(app, 'backward1'))
    it('backward(define/require) case', () => checkOutput(app, 'backward2'))
    it('append case', () => checkOutput(app, 'append'))
    it('append-root case', () => checkOutput(app, 'append-root'))

    it('if case', () => checkOutput(app, 'if'))
    it('if-repeat case', () => checkOutput(app, 'if-repeat'))
    it('if in root element case', () => checkOutput(app, 'if-root'))
    it('repeat in root element case', () => checkOutput(app, 'repeat-root'))
    it('repeat with index case case', () => checkOutput(app, 'repeat-index'))
    it('repeat with array v case', () => checkOutput(app, 'repeat-array-v'))
    it('repeat with array kv case', () => checkOutput(app, 'repeat-array-kv'))
    it('repeat with array no-kv case', () => checkOutput(app, 'repeat-array-no-kv'))
    it('repeat with array non-obj case', () => checkOutput(app, 'repeat-array-non-obj'))
    it('repeat watch case', () => checkOutput(app, 'repeat-watch'))

    it('dynamic type case', () => checkOutput(app, 'dynamic-type'))
    it('dynamic property case', () => checkOutput(app, 'dynamic-property'))

    it('bind subvm', () => checkOutput(app, 'subVm'))
    it('components options', () => checkOutput(app, 'components'))
    it('change data when created', () => checkOutput(app, 'created'))
    it('change data when ready', () => checkOutput(app, 'ready'))

    it('require 3rd', () => checkOutput(app, 'require'))
    it('a correct transformer version', () => checkOutput(app, 'transformer1'))

    it('promise case', () => checkOutput(app, 'promise'))
  })

  // computed
  // append-root-event
  // clear-module
  // repeat-track-by
  // if-refresh
  // if-repeat-refresh
  // reset class style case
  // click
  // inline-click
  // refresh twice
  // a less wrong transformer version
  // a big wrong transformer version
  // input binding
  // use HTML5 timer API
  // use modal API
  // test callNative signals
})
