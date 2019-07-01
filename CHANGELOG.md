# Abstract
## Feature
* Support arm64 & ndk18 on Android platform.
* Android JSC Runtime refactor.
* Android & iOS multi-size screen & rotation support.
* Background JS thread on iOS.
* Log module on iOS and Android to support redirection
* Synchronous call of component methods.
* Unified C++ log system of WeexCore.

## Main Bugfix
* Animation module crash on iOS.
* RTL layout crash on iOS.
* NSTimer not removed by WXTimerModule on iOS.
* Occasionally showing placeholder instead of main image on iOS.
* Animation end progress error on iOS.
* Some NPE issues on Android.
* Closing fd multiple times on Android IPC.
* box-shadow crash protection on Android.
* GPU texture size overflow protection on Android.
* Weexcore.so loading failure problem on Android.

