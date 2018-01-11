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
import fs from 'fs'
import path from 'path'

// load test driver
import {
  Runtime,
  Instance
} from 'weex-vdom-tester'

// load env
import shared from '../../../runtime/shared'
import { Document, Element, Comment } from '../../../runtime/vdom'
import Listener from '../../../runtime/bridge/Listener'
import { TaskCenter, init } from '../../../runtime/bridge/TaskCenter'

// load framework
import * as defaultFramework from '../../../runtime/frameworks/legacy'
import { subversion } from '../../../package.json'

shared.setNativeConsole()

// mock config & global APIs
global.callNativeHandler = function () {}

// init special API called `callAddElement()`
// which is supported temporarily in native render
global.callAddElement = function (id, ref, json, index) {
  return callNativeHandler(id, [{ module: 'dom', method: 'addElement', args: [ref, json, index] }])
}

init()

// create test driver runtime
export function createRuntime () {
  const config = {
    Document, Element, Comment, Listener,
    TaskCenter,
    sendTasks (...args) {
      return callNativeHandler(...args)
    }
  }

  Document.Listener = Listener
  Document.handler = config.sendTasks

  defaultFramework.init(config)

  Object.assign(global, {
    frameworkVersion: subversion.native,
    transformerVersion: subversion.transformer
  })

  return new Runtime(defaultFramework)
}

// create framework instance
export function createApp (runtime) {
  return new Instance(runtime)
}

// read source code from file
export function getCode (filePath) {
  const absolutPath = path.join(__dirname, filePath)
  return fs.readFileSync(absolutPath, 'utf8')
}
