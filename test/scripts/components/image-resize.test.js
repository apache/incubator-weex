'use strict';

var _ = require('macaca-utils');
var assert = require('chai').assert
var wd = require('weex-wd')
var path = require('path');
var os = require('os');
var util = require("../util.js");


var goal = 'image-resize';
var maxW = util.getGETActionWaitTimeMills();
describe('weex '+goal+' test', function () {
  this.timeout(util.getTimeoutMills());
  var driver = util.createDriver(wd);

  beforeEach(function () {
    return util.init(driver)
      .get(util.getPage('/components/'+goal+'.js'))
  });

  afterEach(function () {
    return util.quit(driver);
  })

  it('#1 '+goal + ' test', () => {
    return driver
      .waitForElementByName(goal, maxW, 2000)
      .takeScreenshot()
      .then(imgData => {
        var newImg = new Buffer(imgData, 'base64');
        var screenshotFolder = path.resolve(__dirname, '../../screenshot');
        var oldImgPath = path.join(screenshotFolder, process.env.platform === 'android' ? (goal+'-android.png') : (goal+'-ios.png'));
        var diffImgPath = path.join(screenshotFolder, process.env.platform === 'android' ? (goal+'-android-diff.png') : (goal+'-ios-diff.png') );
        return util.diffImage(oldImgPath, newImg, 1, diffImgPath);
      })
      .then(result => {
        console.log(result)
        assert.isOk(result)
      })

  })
});