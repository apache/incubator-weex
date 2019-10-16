# Weex Apache Source Release
Weex produce SDKs to integrate with iOS/Android/Mobile web applications. This file will cover how to build Weex from command line. You can either use the script we provided or manually build from source step by step.
See `README.md` for further information about the Weex Framework.

Weex SDK includes 3 different SDKs to use in corresponding system/browser:
* Android SDK
* iOS SDK
* [Mobile Web SDK](https://github.com/weexteam/weex-vue-render)

See our [guide in our website](http://weex.apache.org/guide/integrate-to-your-app.html) to learn more about how to integrate Weex SDK into your app.

# Build Environment
The environment required to build weex is categorized by platforms.

## Android
* JDK `1.8+`
* Android SDK Platform 28
  * `$ANDROID_HOME` must be configured by using `export ANDROID_HOME=/path_to_sdk`
  * Normally, you should install [Android Studio](https://developer.android.com/studio) to get Android SDK Platform 28 installed.
* Gradle 4.10+
* NDK `r18`
  * `ANDROID_NDK_HOME` must be configured by using `export ANDROID_NDK_HOME=/path_to_ndk`
  * Higher version of NDK than `r18` isn't not tested yet.
* CMake 3.4.1+

## iOS
* Install [iOS Environment](https://developer.apple.com/library/ios/documentation/IDEs/Conceptual/AppStoreDistributionTutorial/Setup/Setup.html)
* Install [CocoaPods](https://guides.cocoapods.org/using/getting-started.html)
* XCode Command Tools 8.0+

## Mobile
* NodeJS 4.0+

This article was tested in MacOSX system.

# Build All by Script

This script will build Android and iOS SDKs:
> `$ bash scripts/build_from_source.sh `

This may take a while. After that, you can look in `dist/`, `android/sdk/build/outputs/aar` and `ios/sdk/Products` for Web / Android / iOS SDK artifacts.

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
> `cp packages/weex-js-framework/index.min.js ios/sdk/WeexSDK/Resources/main.js`
> `cp packages/weex-js-framework/index.min.js android/sdk/assets/main.js`

## Build Android SDK
1. Install the [Android environment](#android).
2. Execute the following command

    ```
    cd android
    ./gradlew :weex_sdk:clean :weex_sdk:assembleRelease
    ```

3. Output can be found at `android/sdk/build/outputs/aar`

## Build iOS SDK
Execute command below to compile iOS SDK:
> `$ xcodebuild -project ios/sdk/WeexSDK.xcodeproj -target WeexSDK_MTL`

Then you'll find the iOS library(Framework file) under `ios_sdk/Products`.
