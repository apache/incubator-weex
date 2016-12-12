'use strict';

var path = require('path');
var _ = require('macaca-utils');
var xml2map = require('xml2map');

var platform = process.env.platform || 'android';
platform = platform.toLowerCase();

var iOSOpts = {
  platformVersion: '10.0',
  deviceName: 'iPhone 6s Plus',
  platformName: 'iOS',
  reuse:2,
  app: path.join(__dirname, '..', 'app', `${platform}-app-WeexDemo.zip`)
};

var androidOpts = {
  platformName: 'Android',
  //package: 'com.github.android_app_bootstrap',
  //activity: 'com.github.android_app_bootstrap.activity.WelcomeActivity',
  app: path.join(__dirname, '..', `../android/playground/app/build/outputs/apk/playground.apk`)
};


module.exports = require('webdriver-client')(_.merge({}, platform === 'ios' ? iOSOpts : androidOpts));
