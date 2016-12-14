/**
 * @fileOverview
 * The polyfill of BroadcastChannel API.
 * This api can be used to achieve inter-instance communications.
 *
 * https://html.spec.whatwg.org/multipage/comms.html#broadcasting-to-other-browsing-contexts
 */

const channels = {}

/**
 * Mock MessageEvent type
 * @param {string} type
 * @param {object} dict { data, origin, source, ports }
 *
 * This type has been simplified.
 * https://html.spec.whatwg.org/multipage/comms.html#messageevent
 * https://dom.spec.whatwg.org/#interface-event
 */
export function MessageEvent (type, dict = {}) {
  this.type = type || 'message'

  this.data = dict.data || null
  this.origin = dict.origin || ''
  this.source = dict.source || null
  this.ports = dict.ports || []

  // inherit properties
  this.target = null
  this.timeStamp = Date.now()
}

/**
 * Returns a new BroadcastChannel object via which messages for the given
 * channel name can be sent and received.
 * @param {string} name
 */
export function BroadcastChannel (name) {
  // the name property is readonly
  Object.defineProperty(this, 'name', {
    configurable: false,
    enumerable: true,
    writable: false,
    value: String(name)
  })

  this._closed = false
  this.onmessage = null

  if (!channels[this.name]) {
    channels[this.name] = []
  }
  channels[this.name].push(this)
}

/**
 * Sends the given message to other BroadcastChannel objects set up for this channel.
 * @param {any} message
 */
BroadcastChannel.prototype.postMessage = function (message) {
  if (this._closed) {
    throw new Error(`BroadcastChannel "${this.name}" is closed.`)
  }

  const subscribers = channels[this.name]
  if (subscribers && subscribers.length) {
    for (let i = 0; i < subscribers.length; ++i) {
      const member = subscribers[i]

      if (member._closed || member === this) continue

      if (typeof member.onmessage === 'function') {
        member.onmessage(new MessageEvent('message', { data: message }))
      }
    }
  }
}

/**
 * Closes the BroadcastChannel object, opening it up to garbage collection.
 */
BroadcastChannel.prototype.close = function () {
  this._closed = true

  // remove itself from channels.
  if (channels[this.name]) {
    const subscribers = channels[this.name].filter(x => x !== this)
    if (subscribers.length) {
      channels[this.name] = subscribers
    }
    else {
      delete channels[this.name]
    }
  }
}
