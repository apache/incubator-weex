/*eslint no-eval: "off"*/

// fs

import fs from 'fs'
import path from 'path'

function readInput (name) {
  const readpath = path.join(__dirname, './assets', name + '.input')
  return fs.readFileSync(readpath, 'utf8')
}

function readOutput (name) {
  const readpath = path.join(__dirname, './assets', name + '.output')
  return fs.readFileSync(readpath, 'utf8')
}

// test suite

import chai from 'chai'
import sinon from 'sinon'
import sinonChai from 'sinon-chai'

const expect = chai.expect
chai.use(sinonChai)

// test driver

import {
  Runtime,
  Instance
} from 'weex-vdom-tester'

// env

import '../../shared'
import { Document, Element, Comment } from '../../vdom'
import Listener from '../../vdom/listener'

// framework

import * as defaultFramework from '../../default'
import { subversion } from '../../../package.json'

let callNativeHandler = function () {}

const config = {
  Document, Element, Comment, Listener,
  sendTasks (...args) {
    return callNativeHandler(...args)
  }
}

Document.handler = config.sendTasks

defaultFramework.init(config)

Object.assign(global, {
  frameworkVersion: subversion.native,
  transformerVersion: subversion.transformer
})

describe('test input and output', () => {
  let runtime
  let instance

  before(() => {
    runtime = new Runtime(defaultFramework)
    sinon.stub(console, 'info')
    sinon.stub(console, 'warn')
    sinon.stub(console, 'error')
  })

  after(() => {
    console.info.restore()
    console.warn.restore()
    console.error.restore()
  })

  beforeEach(() => {
    instance = new Instance(runtime)
    callNativeHandler = (...args) => instance._target.callNative(...args)
  })

  afterEach(() => {
  })

  it('single case', () => {
    const name = 'foo'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('foo2 case', () => {
    const name = 'foo2'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('foo3 case', () => {
    const name = 'foo3'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('foo4 case', () => {
    const name = 'foo4'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('foo5 case', () => {
    const name = 'foo5'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('foo6 case', () => {
    const name = 'foo6'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('foo7 case', () => {
    const name = 'foo7'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('foo8 case', () => {
    const name = 'foo8'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('foo9 case', () => {
    const name = 'foo9'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('computed case', () => {
    const name = 'computed'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    let actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$refresh({ x: 10 })
    expected.children[0].attr.value = 12
    expected.children[1].attr.value = 12
    actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$refresh({ m: 10 })
    expected.children[0].attr.value = 20
    expected.children[1].attr.value = 20
    actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('computed in repeat case', () => {
    const name = 'computed-in-repeat'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('backward(register/render) case', () => {
    const name = 'backward1'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('backward(define/require) case', () => {
    const name = 'backward2'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('append case', () => {
    const name = 'append'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('append-root case', () => {
    const name = 'append-root'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('if case', () => {
    const name = 'if'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('if in root element case', () => {
    const name = 'if-root'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('repeat with index case', () => {
    const name = 'repeat-index'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('repeat in root element case', () => {
    const name = 'repeat-root'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('repeat with array no-kv case', () => {
    const name = 'repeat-array-no-kv'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('repeat with array v case', () => {
    const name = 'repeat-array-v'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('repeat with array kv case', () => {
    const name = 'repeat-array-kv'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('repeat with array track-by case', () => {
    const name = 'repeat-track-by'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    instance.$refresh({
      titlelist: [
        { text: 'Hello World2' },
        { text: 'Hello World1' }
      ]
    })
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('repeat with array non-obj case', () => {
    const name = 'repeat-array-non-obj'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('repeat-watch case', () => {
    const name = 'repeat-watch'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('if-refresh case', () => {
    const name = 'if-refresh'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    instance.$refresh({ showTitle: false })
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('if-repeat case', () => {
    const name = 'if-repeat'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('if-repeat-refresh case', () => {
    const name = 'if-repeat-refresh'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    instance.$refresh({
      titlelist: [
        { showTitle: false, title: 'Hello World1' },
        { showTitle: true, title: 'Hello World2' },
        { showTitle: true, title: 'Hello World3' }
      ]
    })
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it.skip('reset class style case', () => {
    const name = 'reset-style'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')

    instance.$fireEvent(instance.doc.body.children[0].ref, 'click', {})

    setTimeout(function () {
      expect(instance.getRealRoot()).eql(expected)
    }, 0)

    instance.$destroy()
  })

  it('dynamic type case', () => {
    const name = 'dynamic-type'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('click case', () => {
    const name = 'click'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$fireEvent(instance.doc.body.children[0].ref, 'click', {})

    instance.$destroy()
  })

  it('inline click case', () => {
    const name = 'inline-click'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()

    expect(actual).eql(expected)

    instance.$fireEvent(instance.doc.body.children[0].ref, 'click', {})

    expected.children[0].attr.value = 'Hello World2'
    expect(instance.getRealRoot()).eql(expected)

    instance.$destroy()
  })

  it('bind subvm', () => {
    const name = 'subvm'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('components options', () => {
    const name = 'components'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('refresh twice', () => {
    const name = 'refresh2'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    let actual = instance.getRealRoot()
    expect(actual).eql({
      type: 'container'
    })

    instance.$refresh({ ext: { showbar1: false }})
    instance.$refresh({ ext: { showbar1: true }})
    const expected = eval('(' + outputCode + ')')
    actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('require 3rd', () => {
    const name = 'require'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('a correct transformer version', () => {
    const name = 'transformer1'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('a less wrong transformer version', () => {
    const name = 'transformer2'
    const inputCode = readInput(name)

    const result = instance.$create(inputCode)
    expect(result).to.be.an.instanceof(Error)
    instance.$destroy()
  })

  it('a bigger wrong transformer version', () => {
    const name = 'transformer3'
    const inputCode = readInput(name)

    const result = instance.$create(inputCode)
    expect(result).to.be.an.instanceof(Error)
    instance.$destroy()
  })

  it('change data when created', () => {
    const name = 'created'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('change data when ready', () => {
    const name = 'ready'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('input binding', () => {
    const name = 'input-binding'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    let actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.doc.body.children[0].attr.value = 'abcdefg'
    instance.$fireEvent(instance.doc.body.children[0].ref, 'change', {}, { attrs: { value: 'abcdefg' }})
    expected.children[0].attr.value = 'abcdefg'
    expected.children.push({ type: 'text', attr: { value: '1 - abcdefg' }})
    actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.doc.body.children[0].attr.value = '12345'
    instance.$fireEvent(instance.doc.body.children[0].ref, 'change', {}, { attrs: { value: '12345' }})
    expected.children[0].attr.value = '12345'
    expected.children.push({ type: 'text', attr: { value: '2 - 12345' }})
    actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })

  it('promise case', () => {
    const name = 'promise'
    const inputCode = readInput(name)
    const outputCode = readOutput(name)

    instance.$create(inputCode)
    const expected = eval('(' + outputCode + ')')
    const actual = instance.getRealRoot()
    expect(actual).eql(expected)

    instance.$destroy()
  })
})

describe('test callNative signals', () => {
  // const oriCallNative = defaultFramework.callNative
  // const callNativeSpy = sinon.spy()
  // let runtime
  // let instance

  // function genCallNativeWrapper (count) {
  //   return (name, tasks, cbId) => {
  //     callNativeSpy(tasks)
  //     const length = callNativeSpy.args.length
  //     if (length > count) {
  //       return -1
  //     }
  //     return length
  //   }
  // }

  // before(() => {
  //   runtime = new Runtime(defaultFramework)
  //   sinon.stub(console, 'info')
  //   sinon.stub(console, 'warn')
  //   sinon.stub(console, 'error')
  // })

  // after(() => {
  //   console.info.restore()
  //   console.warn.restore()
  //   console.error.restore()
  // })

  // beforeEach(() => {
  //   instance = new Instance(runtime)
  //   callNativeSpy.reset()
  // })

  // afterEach(() => {
  //   defaultFramework.callNative = oriCallNative
  // })

  // it('signals control', function () {
  //   this.timeout(15000)

  //   const name = 'signals'
  //   const inputCode = readInput(name)

  //   function run (calls) {
  //     callNativeSpy.reset()
  //     defaultFramework.callNative = genCallNativeWrapper(calls)
  //     instance.$create(inputCode)
  //     instance.$destroy()
  //     expect(callNativeSpy.args.length).eql(calls + 2)
  //   }

  //   for (let i = 5; i < 60; i++) {
  //     run(i)
  //   }
  // })

  // it('long signals control', function () {
  //   this.timeout(50000)

  //   const name = 'signals-long'
  //   const inputCode = readInput(name)

  //   function run (calls) {
  //     callNativeSpy.reset()
  //     defaultFramework.callNative = genCallNativeWrapper(calls)
  //     instance.$create(inputCode)
  //     instance.$destroy()
  //     expect(callNativeSpy.args.length).eql(calls + 2)
  //   }

  //   run(10)
  //   run(30)
  //   run(90)
  // })
})
