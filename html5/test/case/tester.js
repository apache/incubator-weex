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

  describe('complex cases', function () {
    let app

    beforeEach(() => {
      app = createApp(runtime)
      callNativeHandler = (...args) => app._target.callNative(...args)
    })

    afterEach(() => {
      callNativeHandler = function () {}
    })

    function readInput (name) {
      return getCode('assets/' + name + '.input')
    }

    function readOutput (name) {
      return getCode('assets/' + name + '.output')
    }

    it('computed case', () => {
      const name = 'computed'
      const inputCode = readInput(name)
      const outputCode = readOutput(name)

      app.$create(inputCode)
      const expected = eval('(' + outputCode + ')')
      expect(app.getRealRoot()).eql(expected)

      app.$refresh({ x: 10 })
      expected.children[0].attr.value = 12
      expected.children[1].attr.value = 12
      expect(app.getRealRoot()).eql(expected)

      app.$refresh({ m: 10 })
      expected.children[0].attr.value = 20
      expected.children[1].attr.value = 20
      expect(app.getRealRoot()).eql(expected)

      app.$destroy()
    })

    it('append-root-event case', () => {
      const name = 'append-root-event'
      const inputCode = readInput(name)
      const outputCode = readOutput(name)

      app.$create(inputCode)
      const expected = eval('(' + outputCode + ')')
      const actual = app.getRealRoot()
      expect(actual).eql(expected)

      app.$fireEvent(app.doc.body.children[0].ref, 'click', {})
      const actual2 = app.getRealRoot()
      expect(actual2.children[0].attr.value).eql(2)

      app.$destroy()
    })

    it('repeat with array track-by case', () => {
      const name = 'repeat-track-by'
      const inputCode = readInput(name)
      const outputCode = readOutput(name)

      app.$create(inputCode)
      app.$refresh({
        titlelist: [
          { text: 'Hello World2' },
          { text: 'Hello World1' }
        ]
      })
      const expected = eval('(' + outputCode + ')')
      expect(app.getRealRoot()).eql(expected)

      app.$destroy()
    })

    it('if-refresh case', () => {
      const name = 'if-refresh'
      const inputCode = readInput(name)
      const outputCode = readOutput(name)

      app.$create(inputCode)
      app.$refresh({ showTitle: false })
      const expected = eval('(' + outputCode + ')')
      expect(app.getRealRoot()).eql(expected)

      app.$destroy()
    })

    it('if-repeat-refresh case', () => {
      const name = 'if-repeat-refresh'
      const inputCode = readInput(name)
      const outputCode = readOutput(name)

      app.$create(inputCode)
      app.$refresh({
        titlelist: [
          { showTitle: false, title: 'Hello World1' },
          { showTitle: true, title: 'Hello World2' },
          { showTitle: true, title: 'Hello World3' }
        ]
      })
      const expected = eval('(' + outputCode + ')')
      expect(app.getRealRoot()).eql(expected)

      app.$destroy()
    })

    it('click case', () => {
      const name = 'click'
      const inputCode = readInput(name)
      const outputCode = readOutput(name)

      app.$create(inputCode)
      const expected = eval('(' + outputCode + ')')
      expect(app.getRealRoot()).eql(expected)

      app.$fireEvent(app.doc.body.children[0].ref, 'click', {})

      app.$destroy()
    })

    it('inline click case', () => {
      const name = 'inline-click'
      const inputCode = readInput(name)
      const outputCode = readOutput(name)

      app.$create(inputCode)
      const expected = eval('(' + outputCode + ')')
      expect(app.getRealRoot()).eql(expected)

      app.$fireEvent(app.doc.body.children[0].ref, 'click', {})

      expected.children[0].attr.value = 'Hello World2'
      expect(app.getRealRoot()).eql(expected)

      app.$destroy()
    })

    it.skip('reset class style case', () => {
      const name = 'reset-style'
      const inputCode = readInput(name)
      const outputCode = readOutput(name)

      app.$create(inputCode)
      const expected = eval('(' + outputCode + ')')

      app.$fireEvent(app.doc.body.children[0].ref, 'click', {})

      setTimeout(function () {
        expect(app.getRealRoot()).eql(expected)
      }, 0)

      app.$destroy()
    })

    it('refresh twice', () => {
      const name = 'refresh2'
      const inputCode = readInput(name)
      const outputCode = readOutput(name)

      app.$create(inputCode)
      expect(app.getRealRoot()).eql({ type: 'container' })

      app.$refresh({ ext: { showbar1: false }})
      app.$refresh({ ext: { showbar1: true }})
      const expected = eval('(' + outputCode + ')')
      expect(app.getRealRoot()).eql(expected)

      app.$destroy()
    })

    it('a less wrong transformer version', () => {
      const name = 'transformer2'
      const inputCode = readInput(name)

      const result = app.$create(inputCode)
      expect(result).to.be.an.instanceof(Error)
      app.$destroy()
    })

    it('a bigger wrong transformer version', () => {
      const name = 'transformer3'
      const inputCode = readInput(name)

      const result = app.$create(inputCode)
      expect(result).to.be.an.instanceof(Error)
      app.$destroy()
    })

    it('input binding', () => {
      const name = 'input-binding'
      const inputCode = readInput(name)
      const outputCode = readOutput(name)

      app.$create(inputCode)
      const expected = eval('(' + outputCode + ')')
      expect(app.getRealRoot()).eql(expected)

      app.doc.body.children[0].attr.value = 'abcdefg'
      app.$fireEvent(app.doc.body.children[0].ref, 'change', {}, { attrs: { value: 'abcdefg' }})
      expected.children[0].attr.value = 'abcdefg'
      expected.children.push({ type: 'text', attr: { value: '1 - abcdefg' }})
      expect(app.getRealRoot()).eql(expected)

      app.doc.body.children[0].attr.value = '12345'
      app.$fireEvent(app.doc.body.children[0].ref, 'change', {}, { attrs: { value: '12345' }})
      expected.children[0].attr.value = '12345'
      expected.children.push({ type: 'text', attr: { value: '2 - 12345' }})
      expect(app.getRealRoot()).eql(expected)

      app.$destroy()
    })
  })

  describe('multi page cases', function () {
    let app

    beforeEach(() => {
      app = createApp(runtime)
      callNativeHandler = (...args) => app._target.callNative(...args)
    })

    afterEach(() => {
      callNativeHandler = function () {}
    })

    function readInput (name) {
      return getCode('assets/' + name + '.input')
    }

    function readOutput (name) {
      return getCode('assets/' + name + '.output')
    }

    it('clear-module case', () => {
      const app2 = createApp(runtime)

      const nameA = 'clear-moduleA'
      const nameB = 'clear-moduleB'
      const inputCodeA = readInput(nameA)
      const outputCodeA = readOutput(nameA)
      const inputCodeB = readInput(nameB)
      const outputCodeB = readOutput(nameB)
      const expectedA = eval('(' + outputCodeA + ')')
      const expectedB = eval('(' + outputCodeB + ')')

      app.$create(inputCodeA)
      app2.$create(inputCodeB)

      expect(app2.getRealRoot()).eql(expectedB)

      app2.$destroy()
      app.$fireEvent(app.doc.body.children[0].ref, 'click', {})
      expect(app.getRealRoot()).eql(expectedA)

      app.$destroy()
    })
  })

  describe('timer & callNative signals', function () {
    let app

    const callNativeSpy = sinon.spy()
    function genCallNativeWrapper (count) {
      return (name, tasks, cbId) => {
        callNativeSpy(tasks)
        const length = callNativeSpy.callCount
        if (length > count) {
          return -1
        }
        return length
      }
    }
    beforeEach(() => {
      app = createApp(runtime)
      callNativeHandler = (...args) => app._target.callNative(...args)
    })

    afterEach(() => {
      callNativeSpy.reset()
      callNativeHandler = function () {}
    })

    function readInput (name) {
      return getCode('assets/' + name + '.input')
    }

    function readOutput (name) {
      return getCode('assets/' + name + '.output')
    }

    it('use HTML5 timer API', function (done) {
      this.timeout(5000)
      const name = 'timer'
      const inputCode = readInput(name)
      const outputCode = readOutput(name)

      app.$create(inputCode)
      const expected = eval('(' + outputCode + ')')
      expect(app.getRealRoot()).eql(expected)

      setTimeout(_ => {
        expect(app.getRealRoot()).eql(expected)
        setTimeout(_ => {
          expected.children[0].attr.value = 'bar'
          expect(app.getRealRoot()).eql(expected)
          app.$destroy()
          done()
        }, 1000)
      }, 1000)
    })

    it('use modal API', function (done) {
      this.timeout(5000)
      const name = 'modal'
      const inputCode = readInput(name)
      const outputCode = readOutput(name)

      app.$create(inputCode)
      const expected = eval('(' + outputCode + ')')
      expect(app.getRealRoot()).eql(expected)

      // the test driver will hold the API callback about 1 sec
      setTimeout(_ => {
        expected.children[0].attr.value = 'bar'
        expect(app.getRealRoot()).eql(expected)
        app.$destroy()
        done()
      }, 1500)
    })

    it('signals control', function () {
      this.timeout(15000)

      const name = 'signals'
      const inputCode = readInput(name)

      function run (calls) {
        callNativeSpy.reset()
        callNativeHandler = genCallNativeWrapper(calls)
        app.$create(inputCode)
        app.$destroy()
        expect(callNativeSpy.callCount).eql(calls + 2)
      }

      for (let i = 5; i < 60; i++) {
        run(i)
      }
    })

    it('long signals control', function () {
      this.timeout(500000)

      const name = 'signals-long'
      const inputCode = readInput(name)

      function run (calls) {
        callNativeSpy.reset()
        callNativeHandler = genCallNativeWrapper(calls)
        app.$create(inputCode)
        app.$destroy()
        expect(callNativeSpy.callCount).eql(calls + 2)
      }

      run(10)
      run(30)
      run(90)
    })
  })
})
