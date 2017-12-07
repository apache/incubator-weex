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
import Toast from '../../../../render/vue/modules/modal/toast'
import Alert from '../../../../render/vue/modules/modal/alert'
import Confirm from '../../../../render/vue/modules/modal/confirm'
import Prompt from '../../../../render/vue/modules/modal/prompt'
import Modal from '../../../../render/vue/modules/modal/modal'
import {
  nodeListToArray
} from '../helper/utils'
/** @test {webSocket module} */
describe('modal module', function () {
  after(function (done) {
    done()
  })
  describe('modal API', function () {
    const modal = new Modal()
    it('should have prototype', function () {
      expect(modal).to.have.property('show')
      expect(modal).to.have.property('destroy')
      expect(modal).to.have.property('createWrap')
      expect(modal).to.have.property('createNode')
      expect(modal).to.have.property('clearNode')
      expect(modal).to.have.property('createNodeContent')
      expect(modal).to.have.property('bindEvents')
    })
    it('wrap style should be weex-modal-wrap when show method is called', function () {
      // Modal
      // assert.strictEqual($modal[0], $el[0], 'collection contains element')
    })
  })
  describe('toast API', function () {
    const toast = Toast
    const config = {
      message: 'Test',
      duration: 1
    }
    it('should have method', function () {
      expect(toast.push).to.be.a('function')
      expect(toast.show).to.be.a('function')
    })
    it('should toast mount on document when push method is called', function (done) {
      const TOAST_WIN_CLASS_NAME = '.weex-toast'
      let $Toast = null
      toast.push(config.message, config.duration)
      $Toast = document.querySelector(TOAST_WIN_CLASS_NAME)
      expect($Toast.innerText, 'should contain Test message').to.be.equal(config.message)
      expect(nodeListToArray($Toast.classList), 'should include hide class').to.include('hide')
      setTimeout(function () {
        expect(nodeListToArray($Toast.classList), 'should remove hide class').to.not.include('hide')
        done()
      }, config.duration * 1000)
    })
    it('call show method while toast queue length < 1', function () {
      const TOAST_WIN_CLASS_NAME = '.weex-toast'
      let $Toast = null
      toast.show()
      $Toast = document.querySelector(TOAST_WIN_CLASS_NAME)
      expect($Toast, 'element should be null when toast quene length < 1').to.be.null
    })
  })
  describe('alert API', function () {
    const config = {
      message: 'Test',
      callback: function () {}
    }
    const alert = new Alert(config)
    it('extends Standard modal API', function () {
      expect(alert).to.have.property('show')
      expect(alert).to.have.property('destroy')
      expect(alert).to.have.property('createWrap')
      expect(alert).to.have.property('createNode')
      expect(alert).to.have.property('clearNode')
      expect(alert).to.have.property('createNodeContent')
      expect(alert).to.have.property('bindEvents')
    })
    it('should have prototype', function () {
      expect(alert).to.have.property('createNodeContent')
      expect(alert).to.have.property('bindEvents')
    })
    it('would alert mount on document when instance is created', function () {
      const ALERT_WEEX_CLASS_NAME = '.weex-alert'
      const ALERT_WEEX_MSG_CLASS_NAME = '.content-msg'
      const BUTTON_CLASS = '.alert-ok'
      let $Alert = null
      $Alert = document.querySelector(ALERT_WEEX_CLASS_NAME)
      const messageShouldBe = $Alert.querySelector(ALERT_WEEX_MSG_CLASS_NAME).innerText
      const button = $Alert.querySelector(BUTTON_CLASS)
      expect(messageShouldBe, 'should contain Test message').to.be.equal(config.message)
      const event = document.createEvent('HTMLEvents')
      event.initEvent('click', false, true)
      button.dispatchEvent(event)
      $Alert = document.querySelector(ALERT_WEEX_CLASS_NAME)
      expect($Alert, 'element would unmount while ok button is clicked').to.be.null
    })
  })
  describe('confirm API', function () {
    let callbackResult = ''
    const config = {
      message: 'Test',
      okTitle: 'ok',
      cancelTitle: 'cancel',
      callback: function (ret) {
        callbackResult = ret
      }
    }
    let confirm = new Confirm(config)
    it('extends Standard modal API', function () {
      expect(confirm).to.have.property('show')
      expect(confirm).to.have.property('destroy')
      expect(confirm).to.have.property('createWrap')
      expect(confirm).to.have.property('createNode')
      expect(confirm).to.have.property('clearNode')
      expect(confirm).to.have.property('createNodeContent')
      expect(confirm).to.have.property('bindEvents')
    })
    it('should have prototype', function () {
      expect(confirm.createNodeContent).to.be.a('function')
      expect(confirm.bindEvents).to.be.a('function')
    })
    it('would confirm mount on document when instance is created', function () {
      const CONFIRM_WEEX_CLASS_NAME = '.weex-confirm'
      const CONFIRM_WEEX_MSG_CLASS_NAME = '.content-msg'
      const OK_BUTTON_CLASS = '.btn-ok'
      const CANCEL_BUTTON_CLASS = '.btn-cancel'
      let $CONFIRM = null
      $CONFIRM = document.querySelector(CONFIRM_WEEX_CLASS_NAME)
      const messageShouldBe = $CONFIRM.querySelector(CONFIRM_WEEX_MSG_CLASS_NAME).innerText
      let button = $CONFIRM.querySelector(OK_BUTTON_CLASS)
      expect(messageShouldBe, 'should contain Test message').to.be.equal(config.message)
      let event = document.createEvent('HTMLEvents')
      event.initEvent('click', false, true)
      button.dispatchEvent(event)
      $CONFIRM = document.querySelector(CONFIRM_WEEX_CLASS_NAME)
      expect($CONFIRM, 'element would unmount while ok button is clicked').to.be.null
      expect(callbackResult, 'callback would return okTitle').to.be.equal(config.okTitle)
      //  reopen confirm for test cancel button
      confirm = new Confirm(config)
      $CONFIRM = document.querySelector(CONFIRM_WEEX_CLASS_NAME)
      button = $CONFIRM.querySelector(CANCEL_BUTTON_CLASS)
      event = document.createEvent('HTMLEvents')
      event.initEvent('click', false, true)
      button.dispatchEvent(event)
      $CONFIRM = document.querySelector(CONFIRM_WEEX_CLASS_NAME)
      expect($CONFIRM, 'element would unmount while ok button is clicked').to.be.null
      expect(callbackResult, 'callback would return okTitle').to.be.equal(config.cancelTitle)
    })
  })
  describe('prompt API', function () {
    let callbackResult = ''
    const config = {
      message: 'Test',
      okTitle: 'ok',
      cancelTitle: 'cancel',
      callback: function (ret) {
        callbackResult = ret
      }
    }
    const inputMessage = 'prompt message'
    let prompt = new Prompt(config)
    it('extends Standard modal API', function () {
      expect(prompt).to.have.property('show')
      expect(prompt).to.have.property('destroy')
      expect(prompt).to.have.property('createWrap')
      expect(prompt).to.have.property('createNode')
      expect(prompt).to.have.property('clearNode')
      expect(prompt).to.have.property('createNodeContent')
      expect(prompt).to.have.property('bindEvents')
    })
    it('should have prototype', function () {
      expect(prompt.createNodeContent).to.be.a('function')
      expect(prompt.bindEvents).to.be.a('function')
    })
    it('would prompt mount on document when instance is created', function () {
      const PROMPT_WEEX_CLASS_NAME = '.weex-prompt'
      const PROMPT_WEEX_MSG_CLASS_NAME = '.content-msg'
      const OK_BUTTON_CLASS = '.btn-ok'
      const CANCEL_BUTTON_CLASS = '.btn-cancel'
      let $Prompt
      $Prompt = document.querySelector(PROMPT_WEEX_CLASS_NAME)
      const messageShouldBe = $Prompt.querySelector(PROMPT_WEEX_MSG_CLASS_NAME).innerText
      let button = $Prompt.querySelector(OK_BUTTON_CLASS)
      expect(messageShouldBe, 'should contain Test message').to.be.equal(config.message)
      let event = document.createEvent('HTMLEvents')
      $Prompt.querySelector('input').value = inputMessage
      event.initEvent('click', false, true)
      button.dispatchEvent(event)
      $Prompt = document.querySelector(PROMPT_WEEX_CLASS_NAME)
      expect($Prompt, 'element would unmount while ok button is clicked').to.be.null
      expect(callbackResult, 'callback would return {result:string,data:string}').to.deep.equal({
        result: config.okTitle,
        data: inputMessage
      })
      //  reopen prompt for test cancel button
      prompt = new Prompt(config)
      $Prompt = document.querySelector(PROMPT_WEEX_CLASS_NAME)
      button = $Prompt.querySelector(CANCEL_BUTTON_CLASS)
      event = document.createEvent('HTMLEvents')
      $Prompt.querySelector('input').value = inputMessage
      event.initEvent('click', false, true)
      button.dispatchEvent(event)
      $Prompt = document.querySelector(PROMPT_WEEX_CLASS_NAME)
      expect($Prompt, 'element would unmount while ok button is clicked').to.be.null
      expect(callbackResult, 'callback would return {result:string,data:string}').to.deep.equal({
        result: config.cancelTitle,
        data: inputMessage
      })
    })
  })
})
