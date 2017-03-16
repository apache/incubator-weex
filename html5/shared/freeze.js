/**
 * Freeze the prototype of javascript build-in objects.
 */
/* istanbul ignore next */
export function freezePrototype () {
  Object.freeze(Object)
  Object.freeze(Array)

  // Object.freeze(Object.prototype)
  freezeObjProto()
  Object.freeze(Array.prototype)
  Object.freeze(String.prototype)
  Object.freeze(Number.prototype)
  Object.freeze(Boolean.prototype)

  Object.freeze(Error.prototype)
  Object.freeze(Date.prototype)
  Object.freeze(RegExp.prototype)
}

function freezeObjProto () {
  freezeObjProtoProperty('__defineGetter__')
  freezeObjProtoProperty('__defineSetter__')
  freezeObjProtoProperty('__lookupGetter__')
  freezeObjProtoProperty('__lookupSetter__')
  freezeObjProtoProperty('constructor')
  freezeObjProtoProperty('hasOwnProperty')
  freezeObjProtoProperty('isPrototypeOf')
  freezeObjProtoProperty('propertyIsEnumerable')
  freezeObjProtoProperty('toLocaleString')
  freezeObjProtoProperty('toString')
  freezeObjProtoProperty('valueOf')
  Object.seal(Object.prototype)
}

function freezeObjProtoProperty (propertyName) {
  const proto = Object.prototype

  if (!proto.hasOwnProperty(propertyName)) {
    return
  }

  const origin = proto[propertyName]
  Object.defineProperty(proto, propertyName, {
    get: function () {
      return origin
    },
    set: function (value) {
      if (this === proto) {
        throw Error(`Cannot assign to read only property ${propertyName} of Object.prototype`)
      }

      Object.defineProperty(this, propertyName, {
        value: value,
        writable: true
      })

      return value
    }
  })
}
