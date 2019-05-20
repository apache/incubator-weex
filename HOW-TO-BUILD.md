# Weex Apache Source Release
Weex produce SDKs to integrate with iOS/Android/Mobile web applications. This file will cover how to build Weex from source. You can either use the script we provided or manually build from source step by step.
See `README.md` for further information about the Weex Framework.

Weex SDK includes 3 different SDKs to use in corresponding system/browser:
* Android SDK
* iOS SDK
* [Mobile Web SDK](https://github.com/weexteam/weex-vue-render)

See our [guide in our website](http://weex.apache.org/guide/integrate-to-your-app.html) to learn more about how to integrate Weex SDK into your app.

## Build Environment
The environment required to build weex is:
* Android SDK:
    * NodeJS 4.0+
    * JDK 1.6+
    * Android SDK(`$ANDROID_HOME` must be set properly)
    * Gradle 2.0+
    * NDK 16 and ndk 13 (**Both of them are needed**)
    * [Ninja 1.8.2+](https://ninja-build.org/)
    * CMake 3.9.0+
* iOS SDK:
    * NodeJS 4.0+
    * XCode & Command Tools 8.0+
* Mobile web SDK:
    * NodeJS 4.0+

This article was tested in MacOSX system.

# Build All by Script

This script will build Android and iOS SDKs:
> `$ bash scripts/build_from_source.sh $NDK13_dir $NDK_16dir`

The `$NDK13_dir` and `$NDK16_dir` should be the directory of NDK 13 and NDK 16, otherwise the Android build would failed.

This may take a while. After that, you can look in `dist/`, `android/sdk/build/output/` and `ios/sdk/Products` for Web/Android/iOS SDK artifacts.

# Build for Platforms

You can build all SDKs with one script as described above, or just build for a single platform step by step.

## Build Javascript Framework
Javascript Framework is required by native SDKs. **So this must be built first.**
Install npm dependencies(You must have node&npm installed):
> `$ npm install --production`

Install build tools:
> `$ npm run install:buildtools`

Build the javascript libraries:
> `$ npm run build:source`

### Before build Native SDK
Move `min` version to Native SDK folder, which will be used by native SDK build.
> `cp packages/weex-js-framework/index.min.js ios_sdk/WeexSDK/Resources/main.js`
> `cp packages/weex-js-framework/index.min.js android_sdk/assets/main.js`

## Build Android SDK

check env

- you have gradle installed, see more details about 'how to install gradle' on the [gradle website](https://gradle.org/install).
- NDK r16  [link](https://developer.android.com/ndk/)
- edit local.propteries (in `Android` dir)

	```
	ndk.dir=/Users/{user}/Library/Android/sdk/ndk-bundle-r16
	sdk.dir=/Users/{user}/Library/Android/sdk
	```

then do buid

> cd Android/sdk

> ../gradlew clean assemble


artifacts path:

` android/sdk/build/outputs/aar`

You can now import the aar file to your android project.

## Build iOS SDK
Execute command below to compile iOS SDK:
> `$ xcodebuild -project ios_sdk/WeexSDK.xcodeproj -target WeexSDK_MTL`

Then you'll find the iOS library(Framework file) under `ios_sdk/Products`.
