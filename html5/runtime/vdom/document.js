import Comment from './comment'
import Element from './element'
import Listener from '../listener'
import { createHandler } from '../handler'
import { addDoc, removeDoc, appendBody, setBody } from './operation'

function updateElement (el, changes) {
  const attrs = changes.attrs || {}
  for (const name in attrs) {
    el.setAttr(name, attrs[name], true)
  }
  const style = changes.style || {}
  for (const name in style) {
    el.setStyle(name, style[name], true)
  }
}

export default function Document (id, url, handler) {
  id = id ? id.toString() : ''
  this.id = id
  this.URL = url

  addDoc(id, this)
  this.nodeMap = {}
  const L = Document.Listener || Listener
  this.listener = new L(id, handler || createHandler(id, Document.handler))
  this.createDocumentElement()
}

Document.handler = null

Document.prototype.destroy = function () {
  delete this.listener
  delete this.nodeMap
  removeDoc(this.id)
}

Document.prototype.open = function () {
  this.listener.batched = false
}

Document.prototype.close = function () {
  this.listener.batched = true
}

Document.prototype.createDocumentElement = function () {
  if (!this.documentElement) {
    const el = new Element('document')
    el.docId = this.id
    el.ownerDocument = this
    el.role = 'documentElement'
    el.depth = 0
    el.ref = '_documentElement'
    this.nodeMap._documentElement = el
    this.documentElement = el
    el.appendChild = (node) => {
      appendBody(this, node)
    }
    el.insertBefore = (node, before) => {
      appendBody(this, node, before)
    }
  }

  return this.documentElement
}

Document.prototype.createBody = function (type, props) {
  if (!this.body) {
    const el = new Element(type, props)
    setBody(this, el)
  }

  return this.body
}

Document.prototype.createElement = function (tagName, props) {
  return new Element(tagName, props)
}

Document.prototype.createComment = function (text) {
  return new Comment(text)
}

Document.prototype.fireEvent = function (el, type, e, domChanges) {
  if (!el) {
    return
  }
  e = e || {}
  e.type = type
  e.target = el
  e.timestamp = Date.now()
  if (domChanges) {
    updateElement(el, domChanges)
  }
  return el.fireEvent(type, e)
}

Document.prototype.getRef = function (ref) {
  return this.nodeMap[ref]
}
