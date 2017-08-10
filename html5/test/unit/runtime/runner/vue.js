import { expect } from 'chai'
import { execute } from './helper'
import { compile } from 'weex-template-compiler'

function toFunction (content) {
  return `function () { ${content} }`
}

function compileAndExecute (template, additional) {
  const { render, staticRenderFns } = compile(template)
  return execute('Vue', `
    new Vue({
      el: '#root',
      render: ${toFunction(render)},
      staticRenderFns: [${staticRenderFns.map(toFunction).join(',')}],
      ${additional}
    })
  `)
}

describe('Vue Examples', () => {
  it('execute Vue code', (done) => {
    execute('Vue', `
      new Vue({
        el: 'body',
        render (h) {
          return h('div', {}, [h('text', {}, 'Hello World')])
        }
      })
    `).then($root => {
      expect($root.type).to.be.equal('div')
      expect($root.children[0].type).to.be.equal('text')
      expect($root.children[0].attr.value).to.be.equal('Hello World')
    }).then(done)
  })

  it('compile and execute', (done) => {
    compileAndExecute(`
      <div>
        <text>Hello World</text>
      </div>
    `).then($root => {
      expect($root.type).to.be.equal('div')
      expect($root.children[0].type).to.be.equal('text')
      expect($root.children[0].attr.value).to.be.equal('Hello World')
    }).then(done)
  })
})
