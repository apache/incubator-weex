# Weex Apache Source Release   
Weex produce SDKs to integrate to iOS/Android/Mobile web applications. This file will cover how to build Weex from source. You can either use the script we provided or manually build from source step by step.   
See `README.md` for further information about Weex Framework.

## Build Environment
The environment required to build weex as follow:   
* NodeJs 4.0+
* Gradle 2.0+
* XCode & Command Tools 8.0+ 

## Build with Script

Run build script:   
> `$ bash scripts/build_from_source.sh`

This's may take a while. After that, you can look into `dist/`, `android/sdk/build/output/` and `ios/sdk/Products` for Web/Android/iOS SDK artifacts.

## Build Step by Step

### Build Javascript Framework and Html5 SDK
Javascript Framework is required by SDKs. **So this must be built first.**  
Install npm dependencies(You must have node&npm installed):   
> `$ npm install --production`

Install build tools:   
> `$ npm run install:buildtools`

Build the javascript libraries:   
> `$ npm run build:source`

The artifacts are under `dist/`.
> ```
> $ ls dist 
> browser.js        browser.min.js    browser.min.js.gz    
> native.js         native.min.js     native.min.js.gz
>```

### Before build Native SDK
Move `min` version to Native SDK folder, which will be used by native SDK build.   
> `cp dist/native.min.js ios_sdk/WeexSDK/Resources/main.js`   
> `cp dist/native.min.js android_sdk/assets/main.js`

### Build Android SDK    
Make sure you have install gradle, see more details about 'how to install gradle' in [gradle website](https://gradle.org/install).   
execute root folder of project   
> `$ gradle wrapper --gradle-version 2.14.1`   

Create a Gradle setting file
> `$ echo 'include ":android_sdk"'>settings.gradle`

Build the SDK   
> `$ ./gradlew :android_sdk:assemble -PasfRelease`

Now, you can see the artifacts under `android_sdk/build/output/`.    
You can now import the aar file to your android project.

### Build iOS SDK 
Execute command below to compile iOS SDK:   
> `$ xcodebuild -project ios_sdk/WeexSDK.xcodeproj -target WeexSDK_MTL`

Then you'll found iOS library(Framework file) under `ios_sdk/Products`.

## Run Apache-Rat

### Download Apache Rat
Download the binary from [Rat's website](https://creadur.apache.org/rat/download_rat.cgi), decompress it somewhere.   
We've tested using v0.12.

### Run Rat against Release files
Just execute in your termial:

> `$ ant -buildfile scripts/rat-ant-build.xml -lib path_to_the_folder_you_place_rat/apache-rat-0.12.jar`

See '`scripts/rat-ant-build.xml`' for details about rules we use.