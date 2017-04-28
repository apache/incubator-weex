/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

function Document () {
  this.refs = {}
}

Document.prototype.createBody = function (config) {
  const doc = this
  const body = this.body = new Element(config)
  this.refs._root = this.body
  if (config.children) {
    config.children.forEach(function (child) {
      appendToDoc(doc, child, body.ref, -1)
    })
  }
}

Document.prototype.addElement = function (parentRef, config, index) {
  appendToDoc(this, config, parentRef, index)
}

function appendToDoc (doc, config, parentRef, index) {
  const parent = doc.refs[parentRef]
  const el = new Element(config)
  doc.refs[el.ref] = el
  el.parentRef = parentRef

  if (index < 0) {
    parent.children.push(el)
  }
  else {
    parent.children.splice(index, 0, el)
  }

  if (config.children) {
    config.children.forEach(function (child) {
      appendToDoc(doc, child, el.ref, -1)
    })
  }
}

Document.prototype.moveElement = function (ref, parentRef, index) {
  const el = this.refs[ref]
  const oldParent = this.refs[el.parentRef]
  const oldIndex = oldParent.children.indexOf(el)

  const parent = this.refs[parentRef]

  if (parent === oldParent && oldIndex <= index) {
    index = index - 1
  }

  oldParent.children.splice(oldIndex, 1)
  parent.children.splice(index, 0, el)
  el.parentRef = parentRef
}

Document.prototype.removeElement = function (ref) {
  removeEl(this, ref)
}

function removeEl (doc, ref) {
  const el = doc.refs[ref]
  const parent = doc.refs[el.parentRef]
  const index = parent.children.indexOf(el)
  const children = el.children || []
  parent.children.splice(index, 1)
  delete doc.refs[ref]
  children.forEach(function (child) {
    removeEl(doc, child.ref)
  })
}

Document.prototype.addEvent = function (ref, type) {
  const el = this.refs[ref]
  const index = el.event.indexOf(type)
  if (index < 0) {
    el.event.push(type)
  }
}

Document.prototype.removeEvent = function (ref, type) {
  const el = this.refs[ref]
  const index = el.event.indexOf(type)
  if (index >= 0) {
    el.event.splice(index, 1)
  }
}

Document.prototype.toJSON = function () {
  const body = this.refs._root
  if (body) {
    return body.toJSON()
  }
  return {}
}

function Element (config) {
  this.ref = config.ref
  this.parentRef = config.parentRef
  this.type = config.type
  this.attr = config.attr || {}
  this.style = config.style || {}
  this.event = config.event || []
  this.children = [] // todo children
}

Element.prototype.toJSON = function () {
  const result = { type: this.type }
  if (Object.keys(this.attr).length > 0) {
    result.attr = this.attr
  }
  if (Object.keys(this.style).length > 0) {
    result.style = this.style
  }
  if (this.event.length > 0) {
    result.event = this.event
  }
  if (this.children.length > 0) {
    result.children = this.children.map(function (child) {
      return child.toJSON()
    })
  }

  return result
}

exports.Document = Document
