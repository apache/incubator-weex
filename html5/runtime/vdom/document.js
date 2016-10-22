import Comment from './comment'
import Element from './element'
import Listener from '../listener'
import { createHandler } from '../handler'
import { extend, addDoc, removeDoc, appendBody, setBody } from './operation'

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

extend(Document.prototype, {
  getRef (ref) {
    return this.nodeMap[ref]
  },

  open () {
    this.listener.batched = false
  },

  close () {
    this.listener.batched = true
  },

  createDocumentElement () {
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
  },

  createBody (type, props) {
    if (!this.body) {
      const el = new Element(type, props)
      setBody(this, el)
    }

    return this.body
  },

  createElement (tagName, props) {
    return new Element(tagName, props)
  },

  createComment (text) {
    return new Comment(text)
  },

  fireEvent (el, type, e, domChanges) {
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
  },

  destroy () {
    delete this.listener
    delete this.nodeMap
    removeDoc(this.id)
  }
})
