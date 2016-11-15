import './arrayFrom'
import './objectAssign'
import './objectSetPrototypeOf'
import './promise'

import { MessageEvent, BroadcastChannel } from './BroadcastChannel'

global.MessageEvent = MessageEvent
global.BroadcastChannel = BroadcastChannel

export * from './console'
export * from './setTimeout'
export * from './freeze'
