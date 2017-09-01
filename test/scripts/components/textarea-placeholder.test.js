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
'use strict';

var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var util = require("../util.js");
var assert = require('chai').assert

var goal = 'textarea-placeholder'

describe('weex '+goal+' test', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);
  var maxW = util.getGETActionWaitTimeMills();
  if (process.env.platform !== 'ios') {
    return;
  }

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/components/'+goal+'.js'))
  });

  afterEach(function () {
    return util.quit(driver);
  })

  it('weex '+goal+' test', () => {
    return driver
      .waitForElementByName(goal,maxW,1000)
      .waitForElementById('textarea',maxW,1000)
      .text().then((text)=>{
        console.log(text)
        assert.notInclude(text,'Hello,I am placeHolder');
        return driver
          .clickScreenById('textarea')
          .waitForElementById('textarea',maxW,1000)
          .clear()
          //TODO placeholder无法捕获
          .waitForElementById('textarea',maxW,1000)
          .sendKeys('hello weex').sleep(1000)
          .waitForElementById('textarea',maxW,1000)
          .text().then((text)=>{
            console.log(text)
            assert.notInclude(text,'Hello,I am placeHolder');
          })
      })
  })
});


