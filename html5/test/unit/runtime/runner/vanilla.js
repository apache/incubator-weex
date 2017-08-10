import { expect } from 'chai'
import { execute } from './helper'

describe('Vanilla Examples', () => {
  it('execute vanilla code', (done) => {
    execute('Vanilla', `
      const body = document.createElement('div')
      document.documentElement.appendChild(body)
    `).then($root => {
      expect($root.type).to.be.equal('div')
    }).then(done)
  })

  it('instert child nodes', (done) => {
    execute('Vanilla', `
      const body = document.createElement('div')
      body.appendChild(document.createElement('text'))
      body.appendChild(document.createElement('image'))
      document.documentElement.appendChild(body)
    `).then($root => {
      expect($root.type).to.be.equal('div')
      expect($root.children[0].type).to.be.equal('text')
      expect($root.children[1].type).to.be.equal('image')
    }).then(done)
  })

  it('add text nodes', (done) => {
    execute('Vanilla', `
      const body = document.createElement('div')
      const text = document.createElement('text')
      text.setAttr('value', 'whatever')
      body.appendChild(text)
      document.documentElement.appendChild(body)
    `).then($root => {
      expect($root.type).to.be.equal('div')
      expect($root.children[0].type).to.be.equal('text')
      expect($root.children[0].attr.value).to.be.equal('whatever')
    }).then(done)
  })
})
