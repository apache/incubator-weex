'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");

describe('weex a test', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/components/hyperlink.js'))
      .waitForElementById("content-inside-a", util.getGETActionWaitTimeMills(), 2000)
  });

  afterEach(function () {
    return util.quit(driver);
  })

  click_a_tag('#1 Click A tag itself', 'a-itself')
  click_a_tag('#2 Click some content inside a', 'content-inside-a')

  function click_a_tag(title, id) {
    it(title, () => {
      return driver
        .elementById(id)
        .click()
        .sleep(2000)
        .waitForElementById('just_text', util.getGETActionWaitTimeMills(), 2000)
        .elementById('just_text')
        .text()
        .then(text => {
          assert.equal(text, 'You just clicked A tag.')
        })
        .elementById('just_text')
        .click()
        .sleep(2000)
    })
  }
});