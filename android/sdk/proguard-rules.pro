# Add project specific ProGuard rules here.
# By default, the flags in this file are appended to flags specified
# in /Users/ouy/Library/AndroidStudio/sdk/tools/proguard/proguard-android.txt
# You can edit the include path and order by changing the proguardFiles
# directive in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# Add any project specific keep options here:

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}
-keep class com.taobao.weex.bridge.** { *; }
-keep class com.taobao.weex.layout.** { *; }
-keep class com.taobao.weex.WXSDKEngine { *; }
-keep class com.taobao.weex.base.SystemMessageHandler { *; }
-dontwarn com.taobao.weex.bridge.**