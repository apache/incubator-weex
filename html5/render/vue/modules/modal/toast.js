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
const queue = []
let isProcessing = false
let toastWin
const TOAST_WIN_CLASS_NAME = 'weex-toast'

const DEFAULT_DURATION = 0.8

function showToastWindow (msg, callback) {
  const handleTransitionEnd = function () {
    toastWin.removeEventListener('transitionend', handleTransitionEnd)
    toastWin.removeEventListener('webkitTransitionEnd', handleTransitionEnd)
    callback && callback()
  }
  if (!toastWin) {
    toastWin = document.createElement('div')
    toastWin.classList.add(TOAST_WIN_CLASS_NAME)
    toastWin.classList.add('hide')
    document.body.appendChild(toastWin)
  }
  toastWin.textContent = msg
  toastWin.addEventListener('transitionend', handleTransitionEnd)
  toastWin.addEventListener('webkitTransitionEnd', handleTransitionEnd)
  setTimeout(function () {
    toastWin.classList.remove('hide')
  }, 0)
}

function hideToastWindow (callback) {
  const handleTransitionEnd = function () {
    toastWin.removeEventListener('transitionend', handleTransitionEnd)
    toastWin.removeEventListener('webkitTransitionEnd', handleTransitionEnd)
    callback && callback()
  }
  if (!toastWin) {
    return
  }
  toastWin.addEventListener('transitionend', handleTransitionEnd)
  toastWin.addEventListener('webkitTransitionEnd', handleTransitionEnd)
  setTimeout(function () {
    toastWin.classList.add('hide')
  }, 0)
}

export default {
  push: function (msg, duration) {
    queue.push({
      msg: msg,
      duration: duration || DEFAULT_DURATION
    })
    this.show()
  },

  show: function () {
    const that = this

    // All messages had been toasted already, so remove the toast window,
    if (!queue.length) {
      toastWin && toastWin.parentNode.removeChild(toastWin)
      toastWin = null
      return
    }

    // the previous toast is not ended yet.
    if (isProcessing) {
      return
    }
    isProcessing = true

    const toastInfo = queue.shift()
    showToastWindow(toastInfo.msg, function () {
      setTimeout(function () {
        hideToastWindow(function () {
          isProcessing = false
          that.show()
        })
      }, toastInfo.duration * 1000)
    })
  }
}
