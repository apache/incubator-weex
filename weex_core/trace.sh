## show c plus plus crash stack
adb logcat | $NDK/ndk-stack -sym  ../android/sdk/build/intermediates/cmake/debug/obj/armeabi-v7a
