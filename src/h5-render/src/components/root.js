'use strict'

var ComponentManager = require('../componentManager')
var Component = require('./component')
var utils = require('../utils')

var scrollable = ['list', 'scroller']

function RootComponent(data, nodeType) {
  var id = data.rootId + '-root'
  var componentManager = ComponentManager.getInstance(data.instanceId)

  // If nodeType is in the downgrades map, just ignore it and
  // replace it with a div element.
  var downgrades = componentManager.weexInstance.downgrades
  this.data = data
  // Return a NodeType instance.
  if (nodeType && nodeType !== 'div' && !downgrades[nodeType]) {
    // If the root component is a Scrolleable Component, then
    // the html and body height should be fixed to the max height
    // of viewport.
    this.fixRootHeight()
    data.type = nodeType
    var cmp = componentManager.createElement(data)
    cmp.node.id = id
    return cmp
  }

  // Otherwise return a common weex-container component,
  // whose node is a div element.
  var node = document.createElement('div')
  this.data = data
  this.node = node

  this.createChildren()
  this.updateAttrs(this.data.attr)
  // issue: when add element to a list in lifetime hook 'ready', the
  // styles is set to the classStyle, not style. This is a issue
  // that jsframework should do something about.
  var classStyle = this.data.classStyle
  classStyle && this.updateStyle(this.data.classStyle)
  this.updateStyle(this.data.style)
  this.bindEvents(this.data.event)
}

RootComponent.prototype = Object.create(Component.prototype)

RootComponent.prototype.fixRootHeight = function () {
  var rootContainer = document.querySelector(
      '#' + this.getComponentManager().weexInstance.rootId
    ) || document.body
  var html = document.documentElement
  var body = document.body
  !html.style.height && (html.style.height = '100%')
  !body.style.height && (body.style.height = '100%')
  !rootContainer.style.height && (rootContainer.style.height = '100%')
  !this.data.style.height
    && (this.data.style.height = window.innerHeight / this.data.scale)
}

module.exports = RootComponent
