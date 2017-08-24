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
'use strict'

var path = require('path');
var os = require('os')
var fs = require('fs')
const BlinkDiff = require('blink-diff');

var platform = process.env.platform || 'android';
const servePort = process.env.serport || 12581
platform = platform.toLowerCase();
var browser = process.env.browser || '';

const isIOS = platform === 'ios';
const isRunInCI = process.env.run_in_ci?true:false;

var iOSOpts = {
  deviceName: 'iPhone 6',
  target: 'ios',
  platformName: 'iOS',
  slowEnv: isRunInCI,
  autoAcceptAlerts:false,
  app: path.join(__dirname, '..', '../ios/playground/build/Debug-iphonesimulator/WeexDemo.app')
};

var androidOpts = {
  platformName: 'Android',
  target: 'android',
  slowEnv: isRunInCI,
  autoAcceptAlerts:false,
  app: path.join(__dirname, '..', '../android/playground/app/build/outputs/apk/playground-debug.apk')
};

var androidChromeOpts = {
  platformName: 'Android',
  target: 'web',
  browserName: 'Chrome'
};

if(isRunInCI){
    console.log("Running in CI Envirment");
}

function getIpAddress(){
    let ifs = os.networkInterfaces()
    let addresses = ['127.0.0.1'];
    for( var i in ifs){
        let interfaces = ifs[i];
        interfaces.forEach((face)=>{
            if(!face.internal && face.family == 'IPv4'){
                addresses.unshift(face.address);
            }
        })
    }
    return addresses[0];
}

function diffImage(imageAPath, imageB, threshold, outputPath) {
  if (!fs.existsSync(imageAPath)) {
    fs.writeFileSync(imageAPath, imageB, 'base64', function(err) {
        console.log(err);
    });
  }
  
  return new Promise((resolve, reject) => {
    var diff = new BlinkDiff({
      imageAPath: imageAPath, // Path
      imageB: imageB,         // Buffer
      thresholdType: BlinkDiff.THRESHOLD_PIXEL,
      threshold: threshold,
      imageOutputPath: outputPath,
      cropImageA:isIOS?{y:128}:{y:242,height:1530},//android: 242 - status bar(72)+navigator bar(170)
      cropImageB:isIOS?{y:128}:{y:242,height:1530}
    });

    diff.run((err, result) => {
      if (err) {
        return reject(err);
      }
      var ifPassed = diff.hasPassed(result.code);
      console.log(ifPassed ? 'Image Comparison Passed' : 'Image Comparison Failed');
      console.log(`Found ${result.differences} pixel differences between two images.`);
      resolve(ifPassed);
    });
  });
}


module.exports = {
    isIos:function(){
      return isIOS;
    },
    getConfig:function(){
        if(browser){
            return androidChromeOpts;
        }
        return isIOS? iOSOpts : androidOpts;
    },
    getDeviceHost:function(){
        return getIpAddress()+":12581";
    },
    getPage:function(name){
        let url
        if(browser){
             url = 'http://'+ getIpAddress()+':'+servePort+'/vue.html?page=/test/build-web'+name
        }else{
            url = 'wxpage://' + getIpAddress()+":"+servePort+"/test/build"+name;
        }
        console.log(url)
        return url
    },
    getTimeoutMills:function(){
        return ( isRunInCI ? 60 : 10 ) * 60 * 1000;
    },
    getGETActionWaitTimeMills:function(){
        return (isRunInCI ? 120 : 5 ) * 1000;
    },
    diffImage, 
    createDriver:function(wd){
        var driver = global._wxDriver;
        if(!driver){
            console.log('Create new driver');
            let driverFactory = wd(this.getConfig())
            driverFactory.addPromiseChainMethod('dragUpAndDown',function(){
                return this
                .getWindowSize()
                .then(size=>{
                    let middleX = size.width * 0.5
                    return this
                    .touch('drag', {fromX:middleX, fromY:size.height*0.7, toX:middleX, toY: size.height*0.3, duration: 1})
                    .sleep(1000)
                    .touch('drag', {fromX:middleX, fromY:size.height*0.3, toX:middleX, toY: size.height*0.7, duration: 1})
                    .sleep(1000)
                })
            })
            driverFactory.addPromiseChainMethod('dragUp',function(distance){
                return this
                .getWindowSize()
                .then(size=>{
                    let middleX = size.width * 0.5
                    let startY = size.height * 0.3
                    return this
                    .touch('drag', {fromX:middleX, fromY:startY+distance, toX:middleX, toY: startY, duration: 1})
                    .sleep(1000)
                })
            })

            //direct : toUp,toDown,toLeft,toRight, dragStartPct: 偏移量，0～1
            driverFactory.addPromiseChainMethod('drag', function(direct,dragStartPct) {
                return this.getWindowSize()
                    .then(size => {
                        let {width,height} = size;
                        if(!direct){direct='toUp';}if(!dragStartPct){dragStartPct=0;}
                        let fromX,toX,fromY,toY;
                        switch (direct) {
                          case 'toUp':
                            fromX = toX = Math.floor(width / 2);
                            fromY = Math.floor(height * 1 / 4 + height * dragStartPct * 3/4);
                            toY = Math.floor(height * 3 / 4 + height * dragStartPct * 1/4);
                            break;
                          case 'toLeft':
                            fromY = toY = Math.floor(height / 2);
                            fromX = Math.floor(width * 1 / 4 + width * dragStartPct * 3 / 4);
                            toX = Math.floor(width * 3 / 4 + width * dragStartPct * 1 / 4);
                            break;
                          case 'toRight':
                            fromY = toY = Math.floor(height / 2);
                            toX = Math.floor(width * 1 / 4 + width * dragStartPct * 3 / 4);
                            fromX = Math.floor(width * 3 / 4 + width * dragStartPct * 1 / 4);
                            break;
                          case 'toDown':
                          default:
                            fromX = toX = Math.floor(width / 2);
                            toY = Math.floor(height * 1 / 4 + height * dragStartPct * 3/4);
                            fromY = Math.floor(height * 3 / 4 + height * dragStartPct * 1/4);
                            break;
                        }
                        return this.touch('drag', {
                                fromX: fromX,fromY: fromY,toX: toX,toY: toY,duration: 0.1
                            })
                            .sleep(500)
                    })
            });

          driverFactory.addPromiseChainMethod('swipeLeft', function (distanceRatio, yRatio) {
                return this
                  .getWindowSize()
                  .then(size => {
                    let y = yRatio * size.height;
                    let startX = size.width * 0.8;
                    let endX = startX - size.width * distanceRatio;
                    return this
                      .touch('drag', {fromX: startX, toX: endX, fromY: y, toY: y, duration: 1})
                      .sleep(1000)
                  })
          })
          driverFactory.addPromiseChainMethod('swipeRight', function (distanceRatio, yRatio) {
            return this
              .getWindowSize()
              .then(size => {
                let y = yRatio * size.height;
                let startX = size.width * 0.2;
                let endX = startX + size.width * distanceRatio;
                return this
                  .touch('drag', {fromX: startX, toX: endX, fromY: y, toY: y, duration: 1})
                  .sleep(1000)
              })
          })
            driver = driverFactory.initPromiseChain();
            driver.configureHttp({
                timeout: 100000
            });
            global._wxDriver = driver;
        }
        
        return driver;
    },
    init:function(driver){
        if(driver._isInit)
            return driver.status()
        else{
            driver._isInit = true;
            return driver.initDriver().sleep(20000) //ios cannot detect at once
        }
    },
    quit:function(driver){
        if(browser)
            return driver.quit()
        return driver.sleep(1000).back().sleep(1000);
    }
}
