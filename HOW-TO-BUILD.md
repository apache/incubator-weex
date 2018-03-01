# Weex Apache Source Release   
Weex produce SDKs to integrate to iOS/Android/Mobile web applications. This file will cover how to build Weex from source. You can either use the script we provided or manually build from source step by step.   
See `README.md` for further information about Weex Framework.

Weex SDK includes 3 different SDKs to use in corresponding system/browser:
* Android SDK
* iOS SDK
* [Mobile Web SDK](https://github.com/weexteam/weex-vue-render)

See our [guide in our website](http://weex.apache.org/guide/integrate-to-your-app.html) to learn more about how to integrate Weex SDK to your app.

## Build Environment
The environment required to build weex as follow:
* Android SDK:
    * NodeJS 4.0+
    * JDK 1.6+
    * Android SDK(`$ANDROID_HOME` must be set properly)
    * Gradle 2.0+
* iOS SDK:
    * NodeJS 4.0+
    * XCode & Command Tools 8.0+
* Mobile web SDK:
    * NodeJS 4.0+

This article was tested in MacOSX system.

# Build All by Script

This script will build Android and iOS SDKs:   
> `$ bash scripts/build_from_source.sh`

This's may take a while. After that, you can look into `dist/`, `android/sdk/build/output/` and `ios/sdk/Products` for Web/Android/iOS SDK artifacts.

# Build for Platforms

You can build all SDKs by one script as described above, or just build for a single platform you want step by step.

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
Make sure you have install gradle, see more details about 'how to install gradle' in [gradle website](https://gradle.org/install).   
execute root folder of project   
> `$ gradle wrapper --gradle-version 2.14.1`   

Create a Gradle setting file
> `$ echo 'include ":android_sdk"'>settings.gradle`

Build the SDK
> `$ ./gradlew :android_sdk:assemble -PasfRelease`

Now, you can see the artifacts under `android_sdk/build/output/`.
You can now import the aar file to your android project.

## Build iOS SDK
Execute command below to compile iOS SDK:
> `$ xcodebuild -project ios_sdk/WeexSDK.xcodeproj -target WeexSDK_MTL`

Then you'll found iOS library(Framework file) under `ios_sdk/Products`.
