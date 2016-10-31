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
 * @param {whatever} message
 *
 * This type has been simplified.
 * https://html.spec.whatwg.org/multipage/comms.html#messageevent
 * https://dom.spec.whatwg.org/#interface-event
 */
function MessageEvent (message) {
  this.data = message
  this.origin = ''
  this.target = null
  this.type = 'message'
  this.timeStamp = Date.now()
}

/**
 * Returns a new BroadcastChannel object via which messages for the given
 * channel name can be sent and received.
 * @param {string} name
 */
export default function BroadcastChannel (name) {
  // the name property is readonly
  Object.defineProperty(this, 'name', {
    configurable: false,
    enumerable: true,
    writable: false,
    value: String(name)
  })

  this.closed = false
  this.onmessage = null

  if (!channels[this.name]) {
    channels[this.name] = []
  }
  channels[this.name].push(this)
}

/**
 * Sends the given message to other BroadcastChannel objects set up for this channel.
 * @param {object} message
 */
BroadcastChannel.prototype.postMessage = function (message) {
  if (this.closed) {
    // should throw an DOMException
    // throw new DOMException('InvalidStateError: This BroadcastChannel is closed.')
    throw new Error('This BroadcastChannel is closed.')
  }

  const subscribers = channels[this.name]
  if (subscribers && subscribers.length) {
    for (let i = 0; i < subscribers.length; ++i) {
      const member = subscribers[i]

      if (member.closed || member === this) continue

      if (typeof member.onmessage === 'function') {
        member.onmessage(new MessageEvent(message))
      }
    }
  }
}

/**
 * Closes the BroadcastChannel object, opening it up to garbage collection.
 */
BroadcastChannel.prototype.close = function () {
  this.closed = true

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
